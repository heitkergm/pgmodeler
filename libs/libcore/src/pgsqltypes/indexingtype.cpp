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

#include "indexingtype.h"

const QStringList IndexingType::type_names {
	"", // Reserved for Class::Null
	"btree",
	"brin",
	"gist",
	"gin",
	"hash",
	"spgist",
};

IndexingType::IndexingType(unsigned type_id)
{
	setType(type_id);
}

IndexingType::IndexingType()
{
	type_idx = Btree;
}

QStringList IndexingType::getTypes()
{
	return TemplateType<IndexingType>::getTypes(type_names);
}

IndexingType::IndexingType(const QString &type_name)
{
	setType(type_name);
}

unsigned IndexingType::setType(unsigned type_id)
{
	return TemplateType<IndexingType>::setType(type_id, type_names);
}

unsigned IndexingType::setType(const QString &type_name)
{
	return TemplateType<IndexingType>::setType(type_name, type_names);
}

QString IndexingType::getTypeName(unsigned type_id)
{
	return TemplateType<IndexingType>::getTypeName(type_id, type_names);
}
