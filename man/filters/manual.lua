--- manual.lua - pandoc filter for the Cppcheck manuals
---
--- Run it after include-files.lua:
---
---   pandoc --lua-filter=filters/include-files.lua --lua-filter=filters/manual.lua ...
---
--- 1. Conditional content, using the Quarto syntax
---    (https://quarto.org/docs/authoring/conditional.html) with profiles:
---
---      ::: {.content-visible when-profile="premium"}
---      Only in the premium manual
---      :::
---
---      ::: {.content-hidden when-profile="premium"}
---      Not in the premium manual
---      :::
---
---    `unless-profile` is also supported. The profile is set by the metadata
---    field `profile`, see manual-opensource.yaml and manual-premium.yaml.
---
--- 2. Included files: a Div with class `.included-file` and an identifier made
---    from the file path (see make-appendix.sh) wraps an included file.
---    - Only the first heading of the file is numbered and listed in the TOC.
---    - Heading identifiers are prefixed with the Div identifier.
---    - Links to the file (e.g. "[x](checkers/uninitvar.md)") point to the Div.
---
--- 3. Raw html `<br/>` is replaced by a line break, raw html is dropped in
---    pdf output.
---
--- 4. Appendices: level 1 headings with class `.appendix` are appendices. In
---    LaTeX output `\appendix` is added before the first one, so the chapter
---    heading says "Appendix A". In other formats the headings in the
---    appendix are numbered "Appendix A", "A.1", "A.2", ...

local profile = nil

local function file_id(target)
  return 'file-' .. target:gsub('[^%w]', '-')
end

local function get_profile(meta)
  if meta.profile then
    profile = pandoc.utils.stringify(meta.profile)
  end
end

local function conditional(div)
  local visible
  if div.classes:includes('content-visible') then
    visible = true
  elseif div.classes:includes('content-hidden') then
    visible = false
  else
    return nil
  end
  for key, _ in pairs(div.attributes) do
    if key ~= 'when-profile' and key ~= 'unless-profile' then
      error('manual.lua: unsupported condition "' .. key .. '"')
    end
  end
  if not profile then
    error('manual.lua: conditional content found but no profile is set, use --metadata-file=manual-<profile>.yaml')
  end
  local when = div.attributes['when-profile']
  local unless = div.attributes['unless-profile']
  local match = (when == nil or when == profile) and (unless == nil or unless ~= profile)
  if match == visible then
    return div.content
  end
  return {}
end

local function included_file(div)
  if not div.classes:includes('included-file') then
    return nil
  end
  local first = true
  return pandoc.walk_block(div, {
    Header = function (h)
      -- Each included file is parsed separately, make the identifiers unique
      if h.identifier ~= '' then
        h.identifier = div.identifier .. '-' .. h.identifier
      end
      if first then
        first = false
      else
        h.classes:insert('unnumbered')
        h.classes:insert('unlisted')
      end
      return h
    end
  })
end

-- Appendix numbering, used for formats other than LaTeX
local appendix_count = 0
local appendix_letter = nil
local section_numbers = {}

local function appendix(h)
  if h.level == 1 then
    if not h.classes:includes('appendix') then
      appendix_letter = nil
      return nil
    end
    appendix_count = appendix_count + 1
    if FORMAT:match('latex') then
      if appendix_count == 1 then
        return { pandoc.RawBlock('latex', '\\appendix'), h }
      end
      return nil
    end
    appendix_letter = string.char(string.byte('A') + appendix_count - 1)
    section_numbers = {}
    h.attributes['number'] = 'Appendix ' .. appendix_letter
    return h
  end
  if appendix_letter and not h.classes:includes('unnumbered') then
    section_numbers[h.level] = (section_numbers[h.level] or 0) + 1
    for level = h.level + 1, 6 do
      section_numbers[level] = nil
    end
    local number = appendix_letter
    for level = 2, h.level do
      number = number .. '.' .. (section_numbers[level] or 0)
    end
    h.attributes['number'] = number
    return h
  end
end

local function document(doc)
  local file_ids = {}
  doc:walk({
    Div = function (div)
      if div.classes:includes('included-file') then
        file_ids[div.identifier] = true
      end
    end
  })

  return doc:walk({
    Link = function (link)
      local id = file_id(link.target)
      if file_ids[id] then
        link.target = '#' .. id
        return link
      end
    end,
    RawInline = function (raw)
      if raw.format == 'html' and raw.text:match('^<br%s*/?>$') then
        return pandoc.LineBreak()
      end
    end,
    Header = appendix
  })
end

return {
  { Meta = get_profile },
  { Div = conditional },
  { Div = included_file },
  { Pandoc = document },
}
