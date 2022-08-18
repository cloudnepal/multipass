/*
 * Copyright (C) 2022 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "create_alias.h"

#include <multipass/format.h>
#include <multipass/platform.h>

multipass::ReturnCode multipass::cmd::create_alias(AliasDict& aliases, const std::string& alias_name,
                                                   const AliasDefinition& alias_definition, std::ostream& cout,
                                                   std::ostream& cerr)
{
    bool empty_before_add = aliases.empty();

    if (!aliases.add_alias(alias_name, alias_definition))
        return ReturnCode::CommandLineError;

    try
    {
        MP_PLATFORM.create_alias_script(alias_name, alias_definition);
    }
    catch (std::runtime_error& e)
    {
        aliases.remove_alias(alias_name);

        cerr << fmt::format("Error when creating script for alias: {}\n", e.what());

        return ReturnCode::CommandLineError;
    }

#ifdef MULTIPASS_PLATFORM_WINDOWS
    QChar separator(';');
#else
    QChar separator(':');
#endif

    // Each element of this list is a folder in the system's path.
    auto path = qEnvironmentVariable("PATH").split(separator);

    auto alias_folder = MP_PLATFORM.get_alias_scripts_folder().absolutePath();

    if (empty_before_add && aliases.size() == 1 && std::find(path.cbegin(), path.cend(), alias_folder) == path.cend())
        cout << MP_PLATFORM.alias_path_message();

    return ReturnCode::Ok;
}
