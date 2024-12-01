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

#ifndef SIMPLE_COLUMNS_WIDGET_H
#define SIMPLE_COLUMNS_WIDGET_H

#include <QWidget>
#include "ui_simplecolumnswidget.h"
#include "pgsqltypewidget.h"
#include "widgets/customtablewidget.h"

class __libgui SimpleColumnsWidget: public QWidget, Ui::SimpleColumnsWidget {
	private:
		Q_OBJECT

		PgSQLTypeWidget *pgsqltype_wgt;

		CustomTableWidget *columns_tab;

		DatabaseModel *model;

		void handleColumn(int row);
		void showColumnData(int row, const QString &name, const QString &type, const QString &alias);

	public:
		explicit SimpleColumnsWidget(QWidget *parent = nullptr);

		void setAttributes(DatabaseModel *model, const std::vector<SimpleColumn> &cols);

		std::vector<SimpleColumn> getColumns();

	private slots:
		void addColumn(int row);
		void updateColumn(int row);
		void editColumn(int row);
		void duplicateColumn(int src_row, int new_row);
};

#endif
