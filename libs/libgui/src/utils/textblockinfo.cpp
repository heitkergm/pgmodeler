/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2024 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

#include "textblockinfo.h"

TextBlockInfo::TextBlockInfo()
{
	reset();
}

void TextBlockInfo::reset()
{
	frag_infos.clear();
	open_group.clear();
}

void TextBlockInfo::addFragmentInfo(const FragmentInfo &f_info)
{
	if(!f_info.isValid())
		return;

	frag_infos.append(f_info);
}

const FragmentInfo *TextBlockInfo::getFragmentInfo(int start, int end)
{
	for(auto &f_info : frag_infos)
	{
		if((start >= f_info.getStart() && start <= f_info.getEnd()) ||
			 (end >= f_info.getStart() && end <= f_info.getEnd()))
			return &f_info;
	}

	return nullptr;
}

void TextBlockInfo::setOpenGroup(const QString &grp)
{
	open_group = grp;
}

QString TextBlockInfo::getOpenGroup()
{
	return open_group;
}

bool TextBlockInfo::isCompletionAllowed(int pos)
{
	for(auto &f_info : frag_infos)
	{
		if(f_info.contains(pos))
			return f_info.isCompletionAllowed();
	}

	return true;
}
