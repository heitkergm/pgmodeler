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

#include "sqltoolwidget.h"
#include "settings/connectionsconfigwidget.h"
#include "guiutilsns.h"
#include "utils/deletableitemdelegate.h"
#include "utilsns.h"
#include "tools/databaseimportform.h"
#include "pgsqlversions.h"

SQLToolWidget::SQLToolWidget(QWidget * parent) : QWidget(parent)
{
	setupUi(this);

	ignore_auto_browse_flag = false;

	DeletableItemDelegate *combo_del = new DeletableItemDelegate(database_cmb, tr("Delete this database"));
	database_cmb->setItemDelegate(combo_del);
	connect(combo_del, &DeletableItemDelegate::s_itemDeleteRequested, this, qOverload<int>(&SQLToolWidget::dropDatabase));

	h_splitter->setSizes({315, 10000});
	h_splitter->handle(1)->installEventFilter(this);
	v_splitter->setSizes({1000, 400});

	sql_exec_corner_btn = new QToolButton(sql_exec_tbw);
	sql_exec_corner_btn->setObjectName("sql_exec_corner_btn");
	sql_exec_corner_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("newtab")));
	sql_exec_corner_btn->setToolTip(tr("Add a new execution tab for the current database (%1)").arg(QKeySequence("Ctrl+T").toString()));

	corner_wgt = new QWidget(sql_exec_tbw);
	corner_wgt->setFixedSize(sql_exec_corner_btn->size());
	corner_wgt->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	sql_exec_tbw->setCornerWidget(corner_wgt);

	corner_wgt_lt = new QVBoxLayout;
	corner_wgt_lt->setContentsMargins(0, 0,
																		corner_wgt->width() / 2,
																		corner_wgt->height());
	corner_wgt->setLayout(corner_wgt_lt);

	QVBoxLayout *vbox=new QVBoxLayout;
	sourcecode_txt=new NumberedTextEditor(sourcecode_gb);
	sourcecode_txt->setReadOnly(true);
	sourcecode_txt->installEventFilter(this);

	sourcecode_hl=new SyntaxHighlighter(sourcecode_txt);
	sourcecode_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	vbox->setContentsMargins(GuiUtilsNs::LtMargin,GuiUtilsNs::LtMargin,GuiUtilsNs::LtMargin,GuiUtilsNs::LtMargin);
	vbox->addWidget(sourcecode_txt);
	sourcecode_gb->setLayout(vbox);

	connect(connections_cmb, &QComboBox::activated, this, __slot(this, SQLToolWidget::connectToServer));
	connect(connections_cmb, &QComboBox::currentIndexChanged, this, [this](int idx) {
		if(idx == 0)
			clearDatabases();
	});

	connect(refresh_tb, &QToolButton::clicked, this, __slot(this, SQLToolWidget::connectToServer));
	connect(database_cmb, &QComboBox::activated, this, __slot(this, SQLToolWidget::browseDatabase));

	connect(disconnect_tb, &QToolButton::clicked, this, &SQLToolWidget::disconnectFromDatabases);
	connect(source_pane_tb, &QToolButton::toggled, sourcecode_gb, &QGroupBox::setVisible);

	connect(databases_tbw, &QTabWidget::tabCloseRequested, this, [this](int idx){
		closeDatabaseExplorer(idx, true);
	});

	connect(sql_exec_tbw, &QTabWidget::tabCloseRequested, this, [this](int idx) {
		closeSQLExecutionTab(idx, true);
	});

	connect(sql_exec_corner_btn, &QToolButton::clicked, this, __slot(this, SQLToolWidget::addSQLExecutionTab));

	connect(databases_tbw, &QTabWidget::currentChanged,	this, [this](){
		DatabaseExplorerWidget *dbexplorer=qobject_cast<DatabaseExplorerWidget *>(databases_tbw->currentWidget());
		QMap<QWidget *, QWidgetList> ::iterator itr=sql_exec_wgts.begin();

		sourcecode_txt->clear();

		if(dbexplorer && dbexplorer->objects_trw->currentItem())
			sourcecode_txt->setPlainText(dbexplorer->objects_trw->currentItem()->
																	 data(DatabaseImportForm::ObjectSource, Qt::UserRole).toString());

		while(itr != sql_exec_wgts.end())
		{
			if(itr.key() != dbexplorer)
			{
				for(auto &wgt : itr.value())
					sql_exec_tbw->removeTab(sql_exec_tbw->indexOf(wgt));
			}
			else
			{
				for(auto &wgt : itr.value())
					sql_exec_tbw->addTab(wgt, dbexplorer->getConnection().getConnectionParam(Connection::ParamDbName));
			}

			itr++;
		}

		disconnect_tb->setEnabled(databases_tbw->count() > 0);
		setCornerButtonPos();
	});
}

SQLToolWidget::~SQLToolWidget()
{
	databases_tbw->blockSignals(true);

	while(databases_tbw->count() > 0)
		closeDatabaseExplorer(0, false);
}

bool SQLToolWidget::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonDblClick &&
		 qobject_cast<QSplitterHandle *>(object) == h_splitter->handle(1))
	{
		if(h_splitter->sizes().at(0) != 0)
			h_splitter->setSizes({0, 10000});
		else
			h_splitter->setSizes({315, 10000});

		return true;
	}
	else if(event->type() == QEvent::MouseButtonPress &&
					dynamic_cast<QMouseEvent *>(event)->button()==Qt::MiddleButton &&
					object == sourcecode_txt &&
					sourcecode_txt->textCursor().hasSelection())
	{
			showSnippet(sourcecode_txt->textCursor().selectedText());
			return true;
	}

	return QWidget::eventFilter(object, event);
}

void SQLToolWidget::setPluginsButtons(const QList<QToolButton *> &list)
{
	plugins_btns = list;
}

void SQLToolWidget::resizeEvent(QResizeEvent *)
{
	setCornerButtonPos();
}

void SQLToolWidget::setCornerButtonPos()
{
	QTabBar *tab_bar = sql_exec_tbw->tabBar();

	if(tab_bar->count() > 0)
	{
		int idx = tab_bar->count() - 1,	px = 0, py = 0;

		QWidget *left_btn = tab_bar->findChild<QWidget *>("ScrollLeftButton"),
					*right_btn = tab_bar->findChild<QWidget *>("ScrollRightButton");

		if(left_btn)
		{
			left_btn->move(left_btn->pos().x(), tab_bar->height() - left_btn->height() - 2);
			right_btn->move(right_btn->pos().x(), tab_bar->height() - right_btn->height() - 2);
		}

		if(left_btn && left_btn->isVisible())
			px = corner_wgt->geometry().left() + 1;
		else
			px = tab_bar->tabRect(idx).right() + 1;

		py = tab_bar->height() - sql_exec_corner_btn->height() - 2;

		sql_exec_corner_btn->raise();
		sql_exec_corner_btn->move(px, py);
		sql_exec_corner_btn->setVisible(true);
	}
	else
		sql_exec_corner_btn->setVisible(false);
}

void SQLToolWidget::updateTabs()
{
	SQLExecutionWidget *sql_exec_wgt=nullptr;

	for(int i=0; i < sql_exec_tbw->count(); i++)
	{
		sql_exec_wgt=dynamic_cast<SQLExecutionWidget *>(sql_exec_tbw->widget(i));
		sql_exec_wgt->sql_cmd_txt->updateLineNumbersSize();
		sql_exec_wgt->sql_cmd_txt->updateLineNumbers();
		sql_exec_wgt->sql_cmd_hl->rehighlight();

		//Forcing the update of the sql history widget (see SQLExecutionWidget::eventFilter)
		sql_exec_wgt->output_tbw->widget(2)->hide();
		sql_exec_wgt->output_tbw->widget(2)->show();
	}
}

void SQLToolWidget::configureSnippets()
{
	SQLExecutionWidget *sql_exec_wgt=nullptr;

	for(int i=0; i < sql_exec_tbw->count(); i++)
	{
		sql_exec_wgt=dynamic_cast<SQLExecutionWidget *>(sql_exec_tbw->widget(i));
		sql_exec_wgt->configureSnippets();
	}
}

void SQLToolWidget::clearDatabases()
{
	database_cmb->clear();
	database_cmb->setEnabled(false);
	refresh_tb->setEnabled(false);
}

void SQLToolWidget::connectToServer()
{
	try
	{
		if(connections_cmb->currentIndex()==connections_cmb->count()-1)
		{
			if(ConnectionsConfigWidget::openConnectionsConfiguration(connections_cmb, true))
				emit s_connectionsUpdateRequest();
		}
		else
		{
			Connection *conn=reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());
			clearDatabases();

			if(conn)
			{
				DatabaseImportForm::listDatabases(*conn, database_cmb);

				if(sender()==connections_cmb && conn->isAutoBrowseDB() && !ignore_auto_browse_flag)
				{
					database_cmb->setCurrentText(conn->getConnectionParam(Connection::ParamDbName));
					browseDatabase();
				}
			}

			database_cmb->setEnabled(database_cmb->count() > 1);
			refresh_tb->setEnabled(database_cmb->isEnabled());
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void SQLToolWidget::disconnectFromDatabases()
{
	try
	{
		Messagebox msg_box;

		msg_box.show(tr("Warning"),
					 tr("<strong>ATTENTION:</strong> Disconnect from all databases will close any opened tab in this view! Do you really want to proceed?"),
					 Messagebox::AlertIcon, Messagebox::YesNoButtons);

		if(msg_box.result()==QDialog::Accepted)
		{
			database_cmb->clear();
			connections_cmb->setEnabled(true);
			refresh_tb->setEnabled(false);

			while(databases_tbw->count() > 0)
			{
				databases_tbw->blockSignals(true);
				closeDatabaseExplorer(0, false);
				databases_tbw->blockSignals(false);
			}

			connections_cmb->setCurrentIndex(0);
			disconnect_tb->setEnabled(false);
			sourcecode_txt->clear();
			setCornerButtonPos();
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, __PRETTY_FUNCTION__, __FILE__, __LINE__);
	}
}

DatabaseExplorerWidget *SQLToolWidget::browseDatabase()
{
	try
	{
		DatabaseExplorerWidget *db_explorer_wgt=nullptr;

		//If the selected database is already being browse do not create another explorer instance
		if(database_cmb->currentIndex() > 0)
		{
			Connection conn=(*reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>()));
			QString maintainance_db=conn.getConnectionParam(Connection::ParamDbName);

			db_explorer_wgt=new DatabaseExplorerWidget;
			db_explorer_wgt->setObjectName(database_cmb->currentText());
			conn.setConnectionParam(Connection::ParamDbName, database_cmb->currentText());
			db_explorer_wgt->setConnection(conn, maintainance_db);
			db_explorer_wgt->listObjects();

			for(auto &btn : plugins_btns)
				db_explorer_wgt->addPluginButton(btn);

			databases_tbw->addTab(db_explorer_wgt, database_cmb->currentText());
			databases_tbw->setTabToolTip(databases_tbw->count() - 1, db_explorer_wgt->getConnection().getConnectionId(true, true));
			databases_tbw->setCurrentWidget(db_explorer_wgt);

			connect(db_explorer_wgt, &DatabaseExplorerWidget::s_sqlExecutionRequested, this, __slot(this, SQLToolWidget::addSQLExecutionTab));
			connect(db_explorer_wgt, &DatabaseExplorerWidget::s_snippetShowRequested, this, &SQLToolWidget::showSnippet);
			connect(db_explorer_wgt, &DatabaseExplorerWidget::s_sourceCodeShowRequested, this, &SQLToolWidget::showSourceCode);
			connect(db_explorer_wgt, &DatabaseExplorerWidget::s_databaseDropRequested, this, qOverload<const QString &>(&SQLToolWidget::dropDatabase));

			connect(attributes_tb, &QToolButton::toggled, db_explorer_wgt->attributes_wgt, &QWidget::setVisible);

			db_explorer_wgt->attributes_wgt->setVisible(attributes_tb->isChecked());

			/* Forcing the signal s_sqlExecutionRequested to be emitted to properly register the
			new tab on the map of sql panes related to the database explorer */
			if(!ignore_auto_browse_flag)
				db_explorer_wgt->runsql_tb->click();
		}

		return db_explorer_wgt;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),__PRETTY_FUNCTION__,__FILE__,__LINE__,&e);
	}
}

SQLExecutionWidget *SQLToolWidget::addSQLExecutionTab(const QString &sql_cmd)
{
	try
	{
		SQLExecutionWidget *sql_exec_wgt = nullptr;
		DatabaseExplorerWidget *db_explorer_wgt=dynamic_cast<DatabaseExplorerWidget *>(databases_tbw->currentWidget());
		Connection conn;

		if(!db_explorer_wgt)
			return nullptr;

		sql_exec_wgt = new SQLExecutionWidget;
		conn = db_explorer_wgt->getConnection();
		sql_exec_wgt->setConnection(conn);
		sql_exec_tbw->addTab(sql_exec_wgt, conn.getConnectionParam(Connection::ParamDbName));
		sql_exec_tbw->setCurrentWidget(sql_exec_wgt);
		sql_exec_tbw->currentWidget()->layout()->setContentsMargins(GuiUtilsNs::LtMargin,GuiUtilsNs::LtMargin,GuiUtilsNs::LtMargin,GuiUtilsNs::LtMargin);
		sql_exec_wgt->sql_cmd_txt->appendPlainText(sql_cmd);
		sql_exec_wgts[db_explorer_wgt].push_back(sql_exec_wgt);

		setCornerButtonPos();

		return sql_exec_wgt;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),__PRETTY_FUNCTION__,__FILE__,__LINE__,&e);
	}
}

void SQLToolWidget::addSQLExecutionTab(const QString &conn_id, const QString &database, const QString &sql_file)
{
	std::map<QString, Connection *> conns;
	SQLExecutionWidget *sql_exec_wgt = nullptr;
	DatabaseExplorerWidget *db_explorer_wgt = nullptr;
	QByteArray buf;

	if(!ConnectionsConfigWidget::getConnection(conn_id))
	{
		throw Exception(tr("Failed to load the file `%1' in SQL tool because the connection ID `%2' was not found!")
										.arg(sql_file).arg(conn_id),
										ErrorCode::Custom,__PRETTY_FUNCTION__,__FILE__,__LINE__);
	}

	buf.append(UtilsNs::loadFile(sql_file));

	// Connect to the server using the provided connection id
	connections_cmb->setCurrentText(conn_id);
	connectToServer();

	// Browse the database and retrive the database explorer instace generated
	database_cmb->setCurrentText(database);
	db_explorer_wgt = browseDatabase();

	/* Now we get the sql execution widget created from the previous operation
	 * in order to load the sql file there */
	sql_exec_wgt = dynamic_cast<SQLExecutionWidget *>(sql_exec_wgts[db_explorer_wgt].at(0));
	sql_exec_wgt->setSQLCommand(buf);
}

void SQLToolWidget::reloadHighlightConfigs()
{
	try
	{
		SQLExecutionWidget *sql_exec_wgt = nullptr;

		for(int i = 0; i < sql_exec_tbw->count(); i++)
		{
			sql_exec_wgt = dynamic_cast<SQLExecutionWidget *>(sql_exec_tbw->widget(i));
			sql_exec_wgt->reloadHighlightConfigs();
		}

		sourcecode_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	}
	catch(Exception &e)
	{
		//Messagebox msgbox;
		//msgbox.show(e);
		Messagebox::error(e, __PRETTY_FUNCTION__, __FILE__, __LINE__);
	}
}

void SQLToolWidget::closeDatabaseExplorer(int idx, bool confirm_close)
{
	DatabaseExplorerWidget *db_explorer = dynamic_cast<DatabaseExplorerWidget *>(databases_tbw->widget(idx));

	/* Display a message box confirming the database explorer tab only if the user
	 * click the close button on the DatabaseExplorerWidget instance */
	if(confirm_close)
	{
		Messagebox msg_box;
		msg_box.show(tr("Warning"),
					 tr("<strong>ATTENTION:</strong> Close the database being browsed will close any opened SQL execution pane related to it! Do you really want to proceed?"),
					 Messagebox::AlertIcon, Messagebox::YesNoButtons);

		if(msg_box.result() != QDialog::Accepted)
			return;
	}

	//Closing sql execution tabs related to the database to be closed
	for(QWidget *wgt : sql_exec_wgts[db_explorer])
	{
		sql_exec_tbw->removeTab(sql_exec_tbw->indexOf(wgt));
		delete wgt;
	}

	sql_exec_wgts.remove(db_explorer);
	databases_tbw->removeTab(idx);

	if(db_explorer)
		delete db_explorer;
}

void SQLToolWidget::ignoreAutoBrowseFlag(bool value)
{
	ignore_auto_browse_flag = value;
}

void SQLToolWidget::closeSQLExecutionTab(int idx, bool confirm_close)
{
	SQLExecutionWidget *sql_exec_wgt = dynamic_cast<SQLExecutionWidget *>(sql_exec_tbw->widget(idx));
	int idx1 = -1;

	//Removing the widget from the list it belongs
	for(auto &itr : sql_exec_wgts)
	{
		idx1 = itr.indexOf(sql_exec_wgt);

		if(idx1 < 0)
			continue;

		if(confirm_close &&	sql_exec_wgt->hasSQLCommand())
		{
			Messagebox msg_box;
			msg_box.show(tr("Warning"),
									 tr("The SQL execution panel contains a typed command! Do you really want to close it?"),
									 Messagebox::AlertIcon, Messagebox::YesNoButtons);

			if(msg_box.result() == QDialog::Rejected)
				return;
		}

		itr.removeAt(idx1);
		break;
	}

	sql_exec_tbw->removeTab(idx);
	setCornerButtonPos();

	if(sql_exec_wgt)
		delete sql_exec_wgt;
}

void SQLToolWidget::showSnippet(const QString &snip)
{
	try
	{
		SQLExecutionWidget *sql_exec_wgt=nullptr;

		if(sql_exec_tbw->count()==0)
			addSQLExecutionTab();

		sql_exec_wgt=dynamic_cast<SQLExecutionWidget *>(sql_exec_tbw->currentWidget());

		if(sql_exec_wgt->sql_cmd_txt->isEnabled())
		{
			QTextCursor cursor=sql_exec_wgt->sql_cmd_txt->textCursor();
			cursor.movePosition(QTextCursor::End);

			sql_exec_wgt->sql_cmd_txt->appendPlainText(snip);
			sql_exec_wgt->sql_cmd_txt->setTextCursor(cursor);
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, __PRETTY_FUNCTION__, __FILE__, __LINE__);
	}
}

void SQLToolWidget::showSourceCode(const QString &source, bool force_display)
{
#ifdef DEMO_VERSION
#warning "DEMO VERSION: SQL code preview truncated."
	if(!source.isEmpty())
	{
		QString trunc_code = source.mid(0, source.size() / 2);
		trunc_code += tr("\n\n-- SQL code purposely truncated at this point in demo version!");
		sourcecode_txt->setPlainText(trunc_code);
	}
#else
	sourcecode_txt->setPlainText(source);
#endif

	if(force_display && !source_pane_tb->isChecked())
		source_pane_tb->setChecked(true);
}

bool SQLToolWidget::hasDatabasesBrowsed()
{
	return (databases_tbw->count() > 0);
}

bool SQLToolWidget::hasSQLExecutionPanels()
{
	for(auto &itr : sql_exec_wgts)
	{
		for(auto &wgt : itr)
		{
			if(qobject_cast<SQLExecutionWidget *>(wgt)->hasSQLCommand())
				return true;
		}
	}

	return false;
}

void SQLToolWidget::dropDatabase(int database_idx)
{
	if(connections_cmb->currentIndex() <= 0 || database_idx <= 0)
		return;

	Connection *tmpl_conn = reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());
	Connection conn = Connection(tmpl_conn->getConnectionParams());
	QString dbname = database_cmb->itemText(database_idx),
			maintanance_db = tmpl_conn->getConnectionParam(Connection::ParamDbName);

	try
	{
		Messagebox msg_box;
		bool allow_force_drop = false;

		conn.connect();

		if(conn.getPgSQLVersion() >= PgSqlVersions::PgSqlVersion130)
		{
			allow_force_drop = true;
			msg_box.setCustomOptionText(tr("Forced database drop"));
			msg_box.setCustomOptionTooltip(tr("<p>If the current user has the proper permissions, this option causes the termination of all existing connections to the target database before dropping it.</p>"));
		}

		msg_box.show(tr("Warning"),
								 tr("<strong>CAUTION:</strong> You are about to drop the entire database <strong>%1</strong> from the server <strong>%2</strong>! All data will be completely wiped out. Do you really want to proceed?")
								.arg(dbname).arg(tmpl_conn->getConnectionId(true)),
								 Messagebox::AlertIcon, Messagebox::YesNoButtons);

		if(msg_box.result() == QDialog::Accepted)
		{
			QString extra_opt;

			if(allow_force_drop && msg_box.isCustomOptionChecked())
				extra_opt = "WITH (FORCE)";

			conn.executeDDLCommand(QString("DROP DATABASE \"%1\" %2;").arg(dbname, extra_opt));
			conn.close();

			//Closing tabs related to the database to be dropped
			for(int i=0; i < databases_tbw->count(); i++)
			{
				if(databases_tbw->tabText(i).remove('&') == dbname)
				{
					closeDatabaseExplorer(i, false);
					i=-1;
				}
			}

			connectToServer();
		}
	}
	catch(Exception &e)
	{
		if(conn.getConnectionParam(Connection::ParamDbName) == maintanance_db)
		{
			/* throw Exception(Exception::getErrorMessage(ErrorCode::DropCurrentDBDefault)
											.arg(dbname, conn.getConnectionParam(Connection::ParamAlias)),
											ErrorCode::DropCurrentDBDefault,__PRETTY_FUNCTION__,__FILE__,__LINE__, &e); */
			Messagebox::error(Exception::getErrorMessage(ErrorCode::DropCurrentDBDefault).arg(dbname, conn.getConnectionParam(Connection::ParamAlias)),
												ErrorCode::DropCurrentDBDefault,__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
		}
		else
			Messagebox::error(e, __PRETTY_FUNCTION__, __FILE__, __LINE__);
	}
}

void SQLToolWidget::dropDatabase(const QString &dbname)
{
	dropDatabase(database_cmb->findText(dbname));
}

QWidgetList SQLToolWidget::getExecutionTabs(DatabaseExplorerWidget *db_expl_wgt)
{
	if(!db_expl_wgt || !sql_exec_wgts.contains(db_expl_wgt))
		return QWidgetList();

	return sql_exec_wgts.value(db_expl_wgt);
}

void SQLToolWidget::moveExecutionTab(DatabaseExplorerWidget *db_expl_wgt, int from_idx, int to_idx)
{
	if(!db_expl_wgt || !sql_exec_wgts.contains(db_expl_wgt) ||
		 from_idx < 0 || to_idx < 0 ||
		 from_idx >= sql_exec_wgts[db_expl_wgt].size() ||
		 to_idx >= sql_exec_wgts[db_expl_wgt].size())
		return;

	sql_exec_wgts[db_expl_wgt].move(from_idx, to_idx);
}
