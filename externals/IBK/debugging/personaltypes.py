############################################################################
#
# Copyright (C) 2016 The Qt Company Ltd.
# Contact: https://www.qt.io/licensing/
#
# This file is part of Qt Creator.
#
# Commercial License Usage
# Licensees holding valid commercial Qt licenses may use this file in
# accordance with the commercial license agreement provided with the
# Software or, alternatively, in accordance with the terms contained in
# a written agreement between you and The Qt Company. For licensing terms
# and conditions see https://www.qt.io/terms-conditions. For further
# information use the contact form at https://www.qt.io/contact-us.
#
# GNU General Public License Usage
# Alternatively, this file may be used under the terms of the GNU
# General Public License version 3 as published by the Free Software
# Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
# included in the packaging of this file. Please review the following
# information to ensure the GNU General Public License requirements will
# be met: https://www.gnu.org/licenses/gpl-3.0.html.
#
############################################################################

# This is a place to add your own dumpers for testing purposes.
# Any contents here will be picked up by GDB, LLDB, and CDB based
# debugging in Qt Creator automatically.

# NOTE: This file will get overwritten when updating Qt Creator.
#
# To add dumpers that don't get overwritten, copy this file here
# to a safe location outside the Qt Creator installation and
# make this location known to Qt Creator using the Debugger >
# Locals & Expressions > Extra Debugging Helpers setting.

# Example to display a simple type
# template<typename U, typename V> struct MapNode
# {
#     U key;
#     V data;
# }
#
# def qdump__MapNode(d, value):
#    d.putValue("This is the value column contents")
#    d.putNumChild(2)
#    if d.isExpanded():
#        with Children(d):
#            # Compact simple case.
#            d.putSubItem("key", value["key"])
#            # Same effect, with more customization possibilities.
#            with SubItem(d, "data")
#                d.putItem("data", value["data"])

# Check http://doc.qt.io/qtcreator/creator-debugging-helpers.html
# for more details or look at qttypes.py, stdtypes.py, boosttypes.py
# for more complex examples.

from dumper import *
from stdtypes import *

######################## Your code below #######################

def qdump__IBK__Path(d, value):
    qdump__std__string(d, value["m_path"])

def qdump__IBK__Unit(d, value):
    qdump__std__string(d, value["m_name"])

def qdump__IBKMK__Vector3D(d, value):
    d.putValue(d.hexencode('[{}, {}, {}]'.format(value["m_x"].value(), value["m_y"].value(), value["m_z"].value())), "utf8:1:1")

def qdump__IBK__Flag(d, value):
    qdump__std__string(d, value["m_name"])
    nameEncoded = d.currentValue.value
    # if empty, nameEncoded == '""' and thus has length 2
    if len(nameEncoded) == 2:
        d.putValue(d.hexencode("empty/false"), "utf8:1:1")
    elif value["m_state"].value() == 0:
        d.putValue(d.hexencode(" = false"), "utf8:1:1")
        d.currentValue.value = nameEncoded + d.currentValue.value
    else:
        d.putValue(d.hexencode(" = true"), "utf8:1:1")
        d.currentValue.value = nameEncoded + d.currentValue.value

