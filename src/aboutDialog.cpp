/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2022 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#include "aboutDialog.h"

#include <QCoreApplication>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QTabWidget>
#include <QTextBrowser>
#include <QTextStream>
#include <QVBoxLayout>

#ifdef Q_OS_MAC
#include <QBitmap>
#endif

NAboutDialog::NAboutDialog(QWidget *parent) : QDialog(parent)
{
    // clang-format off
    QString aboutHtml = QString() +
#ifdef Q_OS_MAC
        "<span style=\"font-size:14pt;\">" +
#else
        "<span style=\"font-size:9pt;\">" +
#endif
            "<b>" +  QCoreApplication::applicationName() + " Music Player</b>" +
            "<br>" +
            "<a href='http://" + QCoreApplication::organizationDomain() + "'>http://" +
                                 QCoreApplication::organizationDomain() + "</a>" +
        "</span><br><br>" +
#ifdef Q_OS_MAC
        "<span style=\"font-size:10pt;\">" +
#else
        "<span style=\"font-size:8pt;\">" +
#endif
            tr("Version: ") + QCoreApplication::applicationVersion() +
            "<br><br>" +
            "Copyright (C) 2010-2018 Sergey Vlasov &lt;sergey@vlasov.me&gt;" +
        "</span>";
    // clang-format on

    setWindowTitle(QObject::tr("About ") + QCoreApplication::applicationName());
    setMaximumSize(0, 0);

    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QTabWidget *tabWidget = new QTabWidget(parent);
    layout->addWidget(tabWidget);

    // about tab >>
    QWidget *aboutTab = new QWidget;
    tabWidget->addTab(aboutTab, tr("Common"));
    QVBoxLayout *aboutTabLayout = new QVBoxLayout;
    aboutTab->setLayout(aboutTabLayout);

    QLabel *iconLabel = new QLabel;
    QPixmap pixmap(":icon-96.png");
    iconLabel->setPixmap(pixmap);
#ifdef Q_OS_MAC
    iconLabel->setMask(pixmap.mask());
#endif

    QHBoxLayout *iconLayout = new QHBoxLayout;
    iconLayout->addStretch();
    iconLayout->addWidget(iconLabel);
    iconLayout->addStretch();
    aboutTabLayout->addLayout(iconLayout);

    QTextBrowser *aboutTextBrowser = new QTextBrowser;
    aboutTextBrowser->setObjectName("aboutTextBrowser");
    aboutTextBrowser->setStyleSheet("background: transparent");
    aboutTextBrowser->setFrameShape(QFrame::NoFrame);
    aboutTextBrowser->setMinimumWidth(350);
    aboutTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    aboutTextBrowser->setOpenExternalLinks(true);
    aboutTextBrowser->setHtml("<center>" + aboutHtml + "</center>");

    aboutTabLayout->addWidget(aboutTextBrowser);
    // << about tab

    // thanks tab >>
    QWidget *thanksTab = new QWidget;
    tabWidget->addTab(thanksTab, tr("Thanks"));
    QVBoxLayout *thanksTabLayout = new QVBoxLayout;
    thanksTabLayout->setContentsMargins(0, 0, 0, 0);
    thanksTab->setLayout(thanksTabLayout);

    QFile thanksFile(":/THANKS");
    thanksFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream thanksStream(&thanksFile);
    QString thanksText = thanksStream.readAll();
    thanksText.replace(QRegExp("(\\w)\\n(\\w)"), "\\1 \\2");
    thanksText.remove("\n\n\n");
    thanksFile.close();

    QTextBrowser *thanksTextBrowser = new QTextBrowser;
    thanksTextBrowser->setText(thanksText);

    thanksTabLayout->addWidget(thanksTextBrowser);
    // << thanks tab

    // changelog tab >>
    QWidget *changelogTab = new QWidget;
    tabWidget->addTab(changelogTab, tr("Changelog"));
    QVBoxLayout *changelogTabLayout = new QVBoxLayout;
    changelogTabLayout->setContentsMargins(0, 0, 0, 0);
    changelogTab->setLayout(changelogTabLayout);

    QFile changelogFile(":/ChangeLog");
    changelogFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream changelogStream(&changelogFile);
    QString changelogHtml = changelogStream.readAll();
    changelogHtml.replace("\n", "<br>\n");
    changelogHtml.replace(QRegExp("(\\*[^<]*)(<br>)"), "<b>\\1</b>\\2");
    changelogFile.close();

    QTextBrowser *changelogTextBrowser = new QTextBrowser;
    changelogTextBrowser->setHtml(changelogHtml);
    changelogTextBrowser->setOpenExternalLinks(true);
    changelogTabLayout->addWidget(changelogTextBrowser);
    // << changelog tab

    // license tab >>
    QWidget *licenseTab = new QWidget;
    tabWidget->addTab(licenseTab, tr("License"));
    QVBoxLayout *licenseTabLayout = new QVBoxLayout;
    licenseTab->setLayout(licenseTabLayout);

    // clang-format off
    QString licenseHtml =
#ifdef Q_OS_MAC
        "<span style=\"font-size:10pt;\">"
#else
        "<span style=\"font-size:8pt;\">"
#endif
            "This program is free software: you can redistribute it and/or modify "
            "it under the terms of the GNU General Public License version 3.0 "
            "as published by the Free Software Foundation.<br>"
            "<br>"
            "This program is distributed in the hope that it will be useful, "
            "but <b>WITHOUT ANY WARRANTY</b>; without even the implied warranty of "
            "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
            "GNU General Public License for more details.<br>"
            "<br>"
            "You should have received a copy of the GNU General Public License "
            "along with this program. If not, see "
            "<a href='http://www.gnu.org/licenses/gpl-3.0.html'>http://www.gnu.org/licenses/gpl-3.0.html</a>."
        "</span>";
    // clang-format on

    QTextBrowser *licenseTextBrowser = new QTextBrowser;
    licenseTextBrowser->setObjectName("licenseTextBrowser");
    licenseTextBrowser->setStyleSheet("background: transparent");
    licenseTextBrowser->setFrameShape(QFrame::NoFrame);
    licenseTextBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    licenseTextBrowser->setOpenExternalLinks(true);
    licenseTextBrowser->setAlignment(Qt::AlignVCenter);
    licenseTextBrowser->setHtml(licenseHtml);

    licenseTabLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
    licenseTabLayout->addWidget(licenseTextBrowser);
    licenseTabLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
    // << license tab

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
}

NAboutDialog::~NAboutDialog() {}

void NAboutDialog::show()
{
    QDialog::show();

    // resize according to content
    foreach (QString objectName, QStringList() << "aboutTextBrowser"
                                               << "licenseTextBrowser") {
        QTextBrowser *textBrowser = parent()->findChild<QTextBrowser *>(objectName);
        QSize textSize = textBrowser->document()->size().toSize();
        textBrowser->setMinimumHeight(textSize.height());
    }
}
