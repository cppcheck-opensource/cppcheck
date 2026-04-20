/*
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "addoninfo.h"

#include "path.h"
#include "utils.h"

#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "json.h"

static std::string getFullPath(const std::string &fileName, const std::string &exename, bool debug = false) {
    if (debug)
        std::cout << "looking for addon '" << fileName << "'" << std::endl;
    if (Path::isFile(fileName))
        return fileName;

    const bool is_abs_path = Path::isAbsolute(fileName);
    if (is_abs_path)
        return "";

    const std::string exepath = Path::getPathFromFilename(exename);
    {
        std::string p = Path::join(exepath, fileName);
        if (debug)
            std::cout << "looking for addon '" << p << "'" << std::endl;
        if (Path::isFile(p))
            return p;
    }
    {
        std::string p = Path::join(exepath, "addons", fileName);
        if (debug)
            std::cout << "looking for addon '" << p << "'" << std::endl;
        if (Path::isFile(p))
            return p;
    }

#ifdef FILESDIR
    {
        std::string p = Path::join(FILESDIR, fileName);
        if (debug)
            std::cout << "looking for addon '" << p << "'" << std::endl;
        if (Path::isFile(p))
            return p;
    }
    {
        std::string p = Path::join(FILESDIR, "addons", fileName);
        if (debug)
            std::cout << "looking for addon '" << p << "'" << std::endl;
        if (Path::isFile(p))
            return p;
    }
#endif
    return "";
}

static std::string parseAddonInfo(AddonInfo& addoninfo, const picojson::value &json, const std::string &fileName, const std::string &exename) {
    const std::string& json_error = picojson::get_last_error();
    if (!json_error.empty()) {
        return "Loading " + fileName + " failed. " + json_error;
    }
    if (!json.is<picojson::object>())
        return "Loading " + fileName + " failed. JSON is not an object.";

    // TODO: remove/complete default value handling for missing fields
    const picojson::object& obj = json.get<picojson::object>();
    {
        const auto it = obj.find("args");
        if (it != obj.cend()) {
            const auto& val = it->second;
            if (!val.is<picojson::array>())
                return "Loading " + fileName + " failed. 'args' must be an array.";
            for (const picojson::value &v : val.get<picojson::array>()) {
                if (!v.is<std::string>())
                    return "Loading " + fileName + " failed. 'args' entry is not a string.";
                addoninfo.args += " " + v.get<std::string>();
            }
        }
    }

    {
        const auto it = obj.find("ctu");
        if (it != obj.cend()) {
            const auto& val = it->second;
            // ctu is specified in the config file
            if (!val.is<bool>())
                return "Loading " + fileName + " failed. 'ctu' must be a boolean.";
            addoninfo.ctu = val.get<bool>();
        }
        else {
            addoninfo.ctu = false;
        }
    }

    {
        const auto it = obj.find("python");
        if (it != obj.cend()) {
            const auto& val = it->second;
            // Python was defined in the config file
            if (!val.is<std::string>()) {
                return "Loading " + fileName +" failed. 'python' must be a string.";
            }
            addoninfo.python = val.get<std::string>();
        }
        else {
            addoninfo.python = "";
        }
    }

    {
        const auto it = obj.find("checkers");
        if (it != obj.cend()) {
            const auto& val = it->second;
            if (!val.is<picojson::array>())
                return "Loading " + fileName + " failed. 'checkers' must be an array.";
            for (const picojson::value &v : val.get<picojson::array>()) {
                if (!v.is<picojson::object>())
                    return "Loading " + fileName + " failed. 'checkers' entry is not an object.";

                const picojson::object& checkerObj = v.get<picojson::object>();
                if (checkerObj.size() == 1) {
                    const std::string c = checkerObj.begin()->first;
                    if (!checkerObj.begin()->second.is<std::string>())
                        return "Loading " + fileName + " failed. 'checkers' entry requirement is not a string.";
                    const std::string req = checkerObj.begin()->second.get<std::string>();
                    addoninfo.checkers.emplace(c, req);
                }
            }
        }
    }

    {
        const auto it = obj.find("executable");
        if (it != obj.cend()) {
            const auto& val = it->second;
            if (!val.is<std::string>())
                return "Loading " + fileName + " failed. 'executable' must be a string.";
            std::string e = val.get<std::string>();
            addoninfo.executable = getFullPath(e, fileName);
            if (addoninfo.executable.empty())
                addoninfo.executable = std::move(e);
            return ""; // <- do not load both "executable" and "script".
        }
    }

    const auto it = obj.find("script");
    if (it == obj.cend())
        return "Loading " + fileName + " failed. 'script' is missing.";

    const auto& val = it->second;
    if (!val.is<std::string>())
        return "Loading " + fileName + " failed. 'script' must be a string.";

    return addoninfo.getAddonInfo(val.get<std::string>(), exename);
}

std::string AddonInfo::getAddonInfo(const std::string &fileName, const std::string &exename, bool debug) {
    if (fileName[0] == '{') {
        picojson::value json;
        const std::string err = picojson::parse(json, fileName);
        (void)err; // TODO: report
        return parseAddonInfo(*this, json, fileName, exename);
    }
    if (fileName.find('.') == std::string::npos)
        return getAddonInfo(fileName + ".py", exename, debug);

    if (endsWith(fileName, ".py")) {
        scriptFile = Path::fromNativeSeparators(getFullPath(fileName, exename, debug));
        if (scriptFile.empty())
            return "Did not find addon " + fileName;

        std::string::size_type pos1 = scriptFile.rfind('/');
        if (pos1 == std::string::npos)
            pos1 = 0;
        else
            pos1++;
        std::string::size_type pos2 = scriptFile.rfind('.');
        if (pos2 < pos1)
            pos2 = std::string::npos;
        name = scriptFile.substr(pos1, pos2 - pos1);

        runScript = getFullPath("runaddon.py", exename);

        return "";
    }

    if (!endsWith(fileName, ".json"))
        return "Failed to open addon " + fileName;

    std::ifstream fin(fileName);
    if (!fin.is_open())
        return "Failed to open " + fileName;
    if (name.empty()) {
        name = Path::fromNativeSeparators(fileName);
        if (name.find('/') != std::string::npos)
            name = name.substr(name.rfind('/') + 1);
    }
    picojson::value json;
    fin >> json;
    return parseAddonInfo(*this, json, fileName, exename);
}
