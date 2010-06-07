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

#include "Lv2Extension.h"
/*
#include "editmode.h"
#include "editormanager.h"
#include "mainwindow.h"
#include "modemanager.h"
#include "fileiconprovider.h"
#include "designmode.h"
*/

#include <extensionsystem/ExtensionManager.h>

#include <QtCore/QtPlugin>
#include <QtCore/QDebug>

using namespace Lv2;
using namespace Lv2::Internal;

Lv2Extension::Lv2Extension()
//  m_mainWindow(new MainWindow), m_editMode(0)
{
}


Lv2Extension::~Lv2Extension()
{
  /*
  if (m_editMode) {
      removeObject(m_editMode);
      delete m_editMode;
  }

  if (m_designMode) {
      removeObject(m_designMode);
      delete m_designMode;
  }

  // delete FileIconProvider singleton
  delete FileIconProvider::instance();

  delete m_mainWindow;
  */
}


void Lv2Extension::parseArguments(const QStringList &arguments)
{
  /*
  for (int i = 0; i < arguments.size() - 1; i++) {
    if (arguments.at(i) == QLatin1String("-color")) {
      const QString colorcode(arguments.at(i + 1));
      m_mainWindow->setOverrideColor(QColor(colorcode));
      i++; // skip the argument
    }
  }
  */
}


bool Lv2Extension::initialize(const QStringList &arguments, QString *errorMessage)
{
  parseArguments(arguments);
  /*
  const bool success = m_mainWindow->init(errorMessage);
  if (success) {
    EditorManager *editorManager = m_mainWindow->editorManager();
    m_editMode = new EditMode(editorManager);
    addObject(m_editMode);

    m_designMode = new DesignMode(editorManager);
    addObject(m_designMode);
  }
  return success;
  */
  return true;
}


void Lv2Extension::extensionsInitialized()
{
  //m_mainWindow->extensionsInitialized();
}


void Lv2Extension::remoteCommand(const QStringList &options, const QStringList &args)
{
  Q_UNUSED(options)
  //m_mainWindow->openFiles(args);
  //m_mainWindow->activateWindow();
}

/*
void CoreExtension::fileOpenRequest(const QString &f)
{
  remoteCommand(QStringList(), QStringList(f));
}
*/

void Lv2Extension::shutdown()
{
  //m_mainWindow->shutdown();
}

EXPORT_EXTENSION(Lv2Extension)

// vim: ts=8 sw=2 sts=2 et sta noai
