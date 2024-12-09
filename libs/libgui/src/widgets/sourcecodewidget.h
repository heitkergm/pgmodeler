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

/**
\ingroup libgui
\class SourceCodeWidget
\brief Implements the operation to visualize object's source code.
*/

#ifndef SOURCE_CODE_WIDGET_H
#define SOURCE_CODE_WIDGET_H

#include "ui_sourcecodewidget.h"
#include "dbobjects/baseobjectwidget.h"
#include "numberedtexteditor.h"
#include "widgets/searchreplacewidget.h"
#include "utils/syntaxhighlighter.h"

class __libgui SourceCodeWidget: public BaseObjectWidget, public Ui::SourceCodeWidget {
	private:
		Q_OBJECT

		NumberedTextEditor *sqlcode_txt, *xmlcode_txt;

		SyntaxHighlighter *hl_sqlcode, *hl_xmlcode;

		SearchReplaceWidget *search_sql_wgt, *search_xml_wgt;

		int prev_pg_ver, prev_code_opt;

		void generateSQLCode();

		void generateXMLCode();

	public:
		SourceCodeWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, BaseObject *object=nullptr);

		/* Forcing the widget to indicate that the handled object is not protected
		even if it IS protected. This will avoid the ok button of the parent dialog
		to be disabled */
		virtual bool isHandledObjectProtected(void){ return false; }

	public slots:
		void applyConfiguration();

	private slots:
		void generateSourceCode(int def_type);
		void setSourceCodeTab(int tab_idx);
		void saveSQLCode();
};

#endif

