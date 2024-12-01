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

#include "checkoptiontype.h"

const QStringList CheckOptionType::type_names {
	"", // Reserved for Class::Null
	"cascaded",
	"local"
};

CheckOptionType::CheckOptionType()
{
	type_idx = Null;
}

QStringList CheckOptionType::getTypes()
{
	return TemplateType<CheckOptionType>::getTypes(type_names);
}

CheckOptionType::CheckOptionType(unsigned type_id)
{
	setType(type_id);
}

CheckOptionType::CheckOptionType(const QString &type_name)
{
	setType(type_name);
}

unsigned CheckOptionType::setType(unsigned type_id)
{
	return TemplateType<CheckOptionType>::setType(type_id, type_names);
}

unsigned CheckOptionType::setType(const QString &type_name)
{
	return TemplateType<CheckOptionType>::setType(type_name, type_names);
}

QString CheckOptionType::getTypeName(unsigned type_id)
{
	return TemplateType<CheckOptionType>::getTypeName(type_id, type_names);
}
