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
\class RelationshipConfigWidget
\brief Implements the operations to manage global settings for relationship objects.
*/

#ifndef RELATIONSHIP_CONFIG_WIDGET_H
#define RELATIONSHIP_CONFIG_WIDGET_H

#include "ui_relationshipconfigwidget.h"
#include "baseconfigwidget.h"

class __libgui RelationshipConfigWidget:  public BaseConfigWidget, public Ui::RelationshipConfigWidget {
	private:
		Q_OBJECT

		static std::map<QString, attribs_map> config_params;

		std::map<QString, attribs_map> patterns;

		virtual void hideEvent(QHideEvent *) override;

	public:
		RelationshipConfigWidget(QWidget * parent = nullptr);

		virtual void saveConfiguration() override;

		virtual void loadConfiguration() override;

		static std::map<QString, attribs_map> getConfigurationParams();

	public slots:
		virtual void applyConfiguration() override;
		virtual void restoreDefaults() override;

	private slots:
		void fillNamePatterns();
		void updatePattern();
		void enableConnModePreview();

};

#endif
