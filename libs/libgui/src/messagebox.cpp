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

#include "messagebox.h"
#include "guiutilsns.h"
#include "utilsns.h"

Messagebox::Messagebox(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);
	this->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	cancelled=false;
	show_errors_tb->setVisible(false);
	custom_option_chk->setVisible(false);

	connect(yes_ok_btn, &QPushButton::clicked, this, &Messagebox::handleYesOkClick);
	connect(no_btn, &QPushButton::clicked, this, &Messagebox::handleNoCancelClick);
	connect(cancel_btn, &QPushButton::clicked, this, &Messagebox::handleNoCancelClick);

	connect(show_errors_tb, &QToolButton::toggled, this, [this](bool checked){
		objs_group_wgt->setCurrentIndex(checked ? 1 : 0);
		resize(baseSize().width() * (checked ? 1.25 : 1),
					 baseSize().height() * (checked ? 3 : 1));
	});
}

void Messagebox::handleYesOkClick()
{
	exceptions_trw->clear();
	accept();
}

void Messagebox::handleNoCancelClick()
{
	exceptions_trw->clear();

	if((sender()==no_btn && !cancel_btn->isVisible()) ||
			(sender()==cancel_btn && !no_btn->isVisible()))
		reject();
	else if(sender()==no_btn && cancel_btn->isVisible())
		reject();
	else if(sender()==cancel_btn && no_btn->isVisible())
	{
		cancelled=true;
		reject();
	}
}

bool Messagebox::isCancelled()
{
	return cancelled;
}

void Messagebox::setCustomOptionText(const QString &text)
{
	custom_option_chk->setVisible(!text.isEmpty());
	custom_option_chk->setText(text);
}

void Messagebox::setCustomOptionTooltip(const QString &tooltip)
{
	custom_option_chk->setToolTip(tooltip);
}

bool Messagebox::isCustomOptionChecked()
{
	return custom_option_chk->isChecked();
}

void Messagebox::show(Exception e, const QString &msg, IconType icon_type, ButtonsId buttons, const QString &yes_lbl, const QString &no_lbl, const QString &cancel_lbl,
											const QString &yes_ico, const QString &no_ico, const QString &cancel_ico)
{
	QString fmt_msg, title;

	raw_info_txt->setPlainText(e.getExceptionsText());
	extra_info_txt->setPlainText(e.getExceptiosExtraInfo());
	stacktrace_tbw->setTabVisible(1, !e.getExceptiosExtraInfo().isEmpty());

	GuiUtilsNs::createExceptionsTree(exceptions_trw, e, nullptr);
	exceptions_trw->expandAll();
	exceptions_trw->scrollToTop();

	if(msg.isEmpty())
		fmt_msg = UtilsNs::formatMessage(e.getErrorMessage());
	else
		fmt_msg = UtilsNs::formatMessage(msg);

	this->show(title, fmt_msg, icon_type, buttons, yes_lbl, no_lbl, cancel_lbl, yes_ico, no_ico, cancel_ico);
}

void Messagebox::show(const QString &msg, IconType icon_type, ButtonsId buttons)
{
	this->show("", msg,  icon_type, buttons);
}

void Messagebox::error(const QString &msg)
{
	Messagebox msgbox;
	msgbox.show(msg, ErrorIcon);
}

void Messagebox::error(const QString &msg, ErrorCode error_code, const QString &method, const QString &file, int line, Exception *e)
{
	Messagebox msgbox;
	msgbox.show(Exception(msg, error_code, method, file, line, e));
}

void Messagebox::error(Exception &e, const QString &method, const QString &file, int line)
{
	error(e.getErrorMessage(), e.getErrorCode(), method, file, line, &e);
}

void Messagebox::alert(const QString &msg, Exception *ex)
{
	Messagebox msgbox;

	if(ex)
		msgbox.show(*ex, msg, AlertIcon);
	else
		msgbox.show(msg, AlertIcon);
}

void Messagebox::info(const QString &msg)
{
	Messagebox msgbox;
	msgbox.show(msg, InfoIcon);
}

void Messagebox::show(const QString &title, const QString &msg, IconType icon_type, ButtonsId buttons, const QString &yes_lbl, const QString &no_lbl,
											const QString &cancel_lbl, const QString &yes_ico, const QString &no_ico, const QString &cancel_ico)
{
	QString icon_name, aux_title=title;
	QWidgetList btns = { yes_ok_btn, no_btn, cancel_btn, show_errors_tb };

	if(!yes_lbl.isEmpty())
		yes_ok_btn->setText(yes_lbl);
	else
	{
		QString btn_txt;

		if(buttons == CloseButton)
			btn_txt = tr("&Close");
		else if(buttons == OkButton)
			btn_txt = tr("&Ok");
		else
			btn_txt = tr("&Yes");

		yes_ok_btn->setText(btn_txt);
	}

	if(!yes_ico.isEmpty())
		yes_ok_btn->setIcon(QIcon(yes_ico));
	else
		yes_ok_btn->setIcon(buttons != CloseButton ? QIcon(GuiUtilsNs::getIconPath("confirm")) : QIcon(GuiUtilsNs::getIconPath("close1")));

	no_btn->setText(!no_lbl.isEmpty() ? no_lbl : tr("&No"));
	no_btn->setIcon(!no_ico.isEmpty() ? QIcon(no_ico) : QIcon(GuiUtilsNs::getIconPath("close1")));

	cancel_btn->setText(!cancel_lbl.isEmpty() ? cancel_lbl : tr("&Cancel"));
	cancel_btn->setIcon(!cancel_ico.isEmpty() ? QIcon(cancel_ico) : QIcon(GuiUtilsNs::getIconPath("cancel")));

	no_btn->setVisible(buttons==YesNoButtons || buttons==AllButtons);
	cancel_btn->setVisible(buttons==OkCancelButtons || buttons==AllButtons);

	for(auto &btn : btns)
	{
		btn->adjustSize();
		btn->setMinimumSize(btn->size());
	}

	if(title.isEmpty())
	{
		switch(icon_type)
		{
		case ErrorIcon:
			aux_title=tr("Error");
			break;

		case AlertIcon:
			aux_title=tr("Alert");
			break;

		case InfoIcon:
			aux_title=tr("Information");
			break;

		case ConfirmIcon:
			aux_title=tr("Confirmation");
			break;

		default:
			break;
		}
	}

	switch(icon_type)
	{
	case ErrorIcon:
		icon_name="error";
		break;

	case InfoIcon:
		icon_name="info";
		break;

	case AlertIcon:
		icon_name="alert";
		break;

	case ConfirmIcon:
		icon_name="question";
		break;

	default:
		icon_name="";
		break;
	}

	cancelled=false;
	icon_lbl->setVisible(!icon_name.isEmpty());

	if(!icon_name.isEmpty())
		icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath(icon_name)));

	msg_lbl->setText(msg);

	setWindowTitle(aux_title);
	objs_group_wgt->setCurrentIndex(0);
	show_errors_tb->setChecked(false);
	show_errors_tb->setVisible(exceptions_trw->topLevelItemCount() > 0);

	double w_factor = 0.25, h_factor = 0.15;
	QSize sz = screen()->size();

	if(sz.width() * w_factor > minimumWidth())
		setMinimumWidth(sz.width() * w_factor);

	setMinimumHeight(sz.height() * h_factor);

	int ln_cnt = QString(msg).replace(QRegularExpression("(<)(br)(/)?(>)",
																											 QRegularExpression::CaseInsensitiveOption),
																		"\n").count('\n');

	if(ln_cnt > 0)
		adjustSize();
	else
		resize(minimumSize());

	setBaseSize(size());
	QDialog::exec();
}

void Messagebox::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
}
