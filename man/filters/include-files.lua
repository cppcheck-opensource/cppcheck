--- include-files.lua – filter to include Markdown files
---
--- Copyright: © 2019–2021 Albert Krewinkel
--- License:   MIT – see LICENSE file for details

-- Module pandoc.path is required and was added in version 2.12
PANDOC_VERSION:must_be_at_least '2.12'

local pandoc = require 'pandoc'
local List = require 'pandoc.List'
local path = require 'pandoc.path'
local system = require 'pandoc.system'

--- Get include auto mode
local include_auto = false
local function get_vars (meta)
  if meta['include-auto'] then
    include_auto = true
  end
end

--- Keep last heading level found
local last_heading_level = 0
local function update_last_level(header)
  last_heading_level = header.level
end

--- Update contents of included file
local function update_contents(blocks, shift_by, include_path)
  local update_contents_filter = {
    -- Shift headings in block list by given number
    Header = function (header)
      if shift_by then
        header.level = header.level + shift_by
      end
      return header
    end,
    -- If link paths are relative then prepend include file path
    Link = function (link)
        if path.is_relative(link.target) and string.sub(path.filename(link.target), 1, 1) ~= '#' and not string.find(link.target, "https?://") then
            link.target = path.normalize(path.join({include_path, link.target}))
        end
        return link
    end,
    -- If image paths are relative then prepend include file path
    Image = function (image)
      if path.is_relative(image.src) then
        image.src = path.normalize(path.join({include_path, image.src}))
      end
      return image
    end,
    -- Update path for include-code-files.lua filter style CodeBlocks
    CodeBlock = function (cb)
      if cb.attributes.include and path.is_relative(cb.attributes.include) then
        cb.attributes.include =
          path.normalize(path.join({include_path, cb.attributes.include}))
        end
      return cb
    end
  }

  return pandoc.walk_block(pandoc.Div(blocks), update_contents_filter).content
end

--- Filter function for code blocks
local transclude
function transclude (cb)
  -- ignore code blocks which are not of class "include".
  if not cb.classes:includes 'include' then
    return
  end

  -- Markdown is used if this is nil.
  local format = cb.attributes['format']

  -- Attributes shift headings
  local shift_heading_level_by = 0
  local shift_input = cb.attributes['shift-heading-level-by']
  if shift_input then
    shift_heading_level_by = tonumber(shift_input)
  else
    if include_auto then
      -- Auto shift headings
      shift_heading_level_by = last_heading_level
    end
  end

  --- keep track of level before recusion
  local buffer_last_heading_level = last_heading_level

  local blocks = List:new()
  for line in cb.text:gmatch('[^\n]+') do
    if line:sub(1,2) ~= '//' then
      local fh = io.open(line)
      if not fh then
        io.stderr:write("Cannot open file " .. line .. " | Skipping includes\n")
      else
        -- read file as the given format with global reader options
        local contents = pandoc.read(
          fh:read '*a',
          format,
          PANDOC_READER_OPTIONS
        ).blocks
        last_heading_level = 0
        -- recursive transclusion
        contents = system.with_working_directory(
            path.directory(line),
            function ()
              return pandoc.walk_block(
                pandoc.Div(contents),
                { Header = update_last_level, CodeBlock = transclude }
              )
            end).content
        --- reset to level before recursion
        last_heading_level = buffer_last_heading_level
        blocks:extend(update_contents(contents, shift_heading_level_by,
                                      path.directory(line)))
        fh:close()
      end
    end
  end
  return blocks
end

return {
  { Meta = get_vars },
  { Header = update_last_level, CodeBlock = transclude }
}
