/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "plugindetailsview.h"
#include "ui_plugindetailsview.h"
#include "plugininfo.h"

#include <QtCore/QDir>

/*!
    \class ExtensionSystem::PluginDetailsView
    \brief Widget that displays the contents of a PluginInfo.

    Can be used for integration in the application that
    uses the plugin manager.

    \sa ExtensionSystem::PluginView
*/

using namespace ExtensionSystem;

/*!
    \fn PluginDetailsView::PluginDetailsView(QWidget *parent)
    Constructs a new view with given \a parent widget.
*/
PluginDetailsView::PluginDetailsView(QWidget *parent)
	: QWidget(parent),
          m_ui(new Internal::Ui::PluginDetailsView())
{
    m_ui->setupUi(this);
}

/*!
    \fn PluginDetailsView::~PluginDetailsView()
    \internal
*/
PluginDetailsView::~PluginDetailsView()
{
    delete m_ui;
}

/*!
    \fn void PluginDetailsView::update(PluginInfo *info)
    Reads the given \a info and displays its values
    in this PluginDetailsView.
*/
void PluginDetailsView::update(PluginInfo *info)
{
    m_ui->name->setText(info->name());
    m_ui->version->setText(info->version());
    m_ui->compatVersion->setText(info->compatVersion());
    m_ui->vendor->setText(info->vendor());
    const QString link = QString::fromLatin1("<a href=\"%1\">%1</a>").arg(info->url());
    m_ui->url->setText(link);
    QString component = tr("None");
    if (!info->category().isEmpty())
        component = info->category();
    m_ui->component->setText(component);
    m_ui->location->setText(QDir::toNativeSeparators(info->filePath()));
    m_ui->description->setText(info->description());
    m_ui->copyright->setText(info->copyright());
    m_ui->license->setText(info->license());
    QStringList depStrings;
    foreach (const PluginDependency &dep, info->dependencies()) {
        QString depString = dep.name;
        depString += QLatin1String(" (");
        depString += dep.version;
        depString += QLatin1Char(')');
        depStrings.append(depString);
    }
    m_ui->dependencies->addItems(depStrings);
}
