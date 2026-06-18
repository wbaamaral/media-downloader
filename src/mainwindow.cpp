/*
 *
 *  Copyright (c) 2021
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utility.h"
#include "tabmanager.h"

#include "context.hpp"
#include "settings.h"
#include "translator.h"

#include <QFrame>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSizePolicy>
#include <QScrollArea>
#include <QTabWidget>
#include <QSplitter>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <initializer_list>
#include <algorithm>
#include <csignal>

namespace
{
	struct ScrollPage
	{
		QWidget *content = nullptr ;
		QVBoxLayout *layout = nullptr ;
	} ;

	ScrollPage createScrollPage( QWidget * page )
	{
		ScrollPage scrollPage ;

		auto rootLayout = new QVBoxLayout( page ) ;
		rootLayout->setContentsMargins( 0,0,0,0 ) ;
		rootLayout->setSpacing( 0 ) ;

		auto scrollArea = new QScrollArea( page ) ;
		scrollArea->setFrameShape( QFrame::NoFrame ) ;
		scrollArea->setWidgetResizable( true ) ;
		scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded ) ;
		scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded ) ;

		scrollPage.content = new QWidget( scrollArea ) ;
		scrollPage.layout = new QVBoxLayout( scrollPage.content ) ;
		scrollPage.layout->setContentsMargins( 14,14,14,14 ) ;
		scrollPage.layout->setSpacing( 12 ) ;

		scrollArea->setWidget( scrollPage.content ) ;
		rootLayout->addWidget( scrollArea ) ;

		return scrollPage ;
	}

	QGroupBox * createSection( const QString& title,QWidget * parent )
	{
		auto group = new QGroupBox( title,parent ) ;
		group->setFlat( false ) ;
		return group ;
	}

	void wrapTabWidgetPages( QTabWidget * tabWidget )
	{
		if( !tabWidget ){
			return ;
		}

	for( int i = 0 ; i < tabWidget->count() ; ++i ){

		auto page = tabWidget->widget( i ) ;

		if( !page || page->layout() ){
			continue ;
		}

		const auto directChildren = page->findChildren< QWidget * >( QString(),Qt::FindDirectChildrenOnly ) ;

		auto scrollArea = new QScrollArea( page ) ;
		scrollArea->setObjectName( page->objectName() + "_scrollArea" ) ;
		scrollArea->setFrameShape( QFrame::NoFrame ) ;
		scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded ) ;
		scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded ) ;
		scrollArea->setWidgetResizable( true ) ;

		auto container = new QWidget ;
		container->setObjectName( page->objectName() + "_container" ) ;

			auto maxWidth = 0 ;
			auto maxHeight = 0 ;
		for( auto child : directChildren ){

				if( !child ){
					continue ;
				}

				const auto geometry = child->geometry() ;
				child->setParent( container ) ;
				child->setGeometry( geometry ) ;

				maxWidth = std::max( maxWidth,geometry.right() ) ;
				maxHeight = std::max( maxHeight,geometry.bottom() ) ;
			}

			container->setMinimumSize( maxWidth + 20,maxHeight + 20 ) ;
			scrollArea->setWidget( container ) ;

			auto layout = new QVBoxLayout( page ) ;
			layout->setContentsMargins( 0, 0, 0, 0 ) ;
			layout->addWidget( scrollArea ) ;
		}
	}

	void setupConfigureGeneralOptionsLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabWidgetConfigureGeneralOptions ;

		if( !page || page->layout() ){
			return ;
		}

		auto scrollPage = createScrollPage( page ) ;
		auto outer = scrollPage.layout ;

		auto header = createSection( QObject::tr( "Core Settings" ),scrollPage.content ) ;
		auto grid = new QGridLayout( header ) ;
		grid->setContentsMargins( 12,12,12,12 ) ;
		grid->setHorizontalSpacing( 10 ) ;
		grid->setVerticalSpacing( 6 ) ;
		grid->setColumnStretch( 1,1 ) ;

		grid->addWidget( ui.labelConfigureLanguage,0,0 ) ;
		grid->addWidget( ui.cbConfigureLanguage,0,1,1,2 ) ;

		grid->addWidget( ui.labelConfigureTheme,1,0 ) ;
		grid->addWidget( ui.comboBoxConfigureDarkTheme,1,1 ) ;
		grid->addWidget( ui.pbOpenThemeFolder,1,2 ) ;

		grid->addWidget( ui.labelConfigureDownloadPath,2,0 ) ;
		grid->addWidget( ui.lineEditConfigureDownloadPath,2,1 ) ;
		grid->addWidget( ui.pbConfigureDownloadPath,2,2 ) ;

		grid->addWidget( ui.labelMaximumConcurrentDownloads,3,0 ) ;
		grid->addWidget( ui.lineEditConfigureMaximuConcurrentDownloads,3,1,1,2 ) ;

		grid->addWidget( ui.labelActionsAtStartup,4,0 ) ;
		grid->addWidget( ui.comboBoxActionsWhenStarting,4,1,1,2 ) ;

		outer->addWidget( header ) ;

		auto behavior = createSection( QObject::tr( "Behavior" ),scrollPage.content ) ;
		auto behaviorGrid = new QGridLayout( behavior ) ;
		behaviorGrid->setContentsMargins( 12,12,12,12 ) ;
		behaviorGrid->setHorizontalSpacing( 14 ) ;
		behaviorGrid->setVerticalSpacing( 6 ) ;
		behaviorGrid->setColumnStretch( 0,1 ) ;
		behaviorGrid->setColumnStretch( 1,1 ) ;

		behaviorGrid->addWidget( ui.cbShowTrayIcon,0,0 ) ;
		behaviorGrid->addWidget( ui.cbAutoHideDownloadCompleted,0,1 ) ;
		behaviorGrid->addWidget( ui.cbAutoSaveNotDownloadedMedia,1,0 ) ;
		behaviorGrid->addWidget( ui.cbLibraryTabEnable,1,1 ) ;
		behaviorGrid->addWidget( ui.cbconfigureAutoDownload,2,0 ) ;
		behaviorGrid->addWidget( ui.cbConfigureShowMetaDataInBatchDownloader,2,1 ) ;
		behaviorGrid->addWidget( ui.cbConfigureNotifyWhenDownloadComplete,3,0 ) ;
		behaviorGrid->addWidget( ui.cbConfigureNotifyWhenAllDownloadCompltes,3,1 ) ;

		outer->addWidget( behavior ) ;

		auto engineSource = createSection( QObject::tr( "Engine Source" ),scrollPage.content ) ;
		auto engineSourceLayout = new QVBoxLayout( engineSource ) ;
		engineSourceLayout->setContentsMargins( 12,12,12,12 ) ;
		engineSourceLayout->setSpacing( 8 ) ;
		engineSourceLayout->addWidget( ui.cbConfigureUseSystemEngine ) ;
		engineSourceLayout->addWidget( ui.cbConfigureUseSystemSupportingEngine ) ;

		outer->addWidget( engineSource ) ;
		outer->addStretch( 1 ) ;
	}

	void styleActionButton( QPushButton * button )
	{
		if( !button ){
			return ;
		}

		button->setMinimumWidth( 108 ) ;
		button->setMinimumHeight( 34 ) ;
		button->setStyleSheet(
			"QPushButton {"
			" border: 1px solid palette(mid);"
			" border-radius: 12px;"
			" padding: 8px 18px;"
			" background-color: palette(button);"
			"}"
			"QPushButton:hover {"
			" background-color: palette(light);"
			"}"
			"QPushButton:pressed {"
			" background-color: palette(mid);"
			"}"
		) ;
	}

	QFrame * createActionBar( QWidget * page )
	{
		auto bar = new QFrame( page ) ;
		bar->setObjectName( "actionBar" ) ;
		bar->setFrameShape( QFrame::StyledPanel ) ;
		bar->setSizePolicy( QSizePolicy::Expanding,QSizePolicy::Fixed ) ;
		bar->setStyleSheet(
			"QFrame#actionBar {"
			" border-radius: 14px;"
			" background-color: palette(window);"
			" border: 1px solid palette(mid);"
			"}"
		) ;

		auto layout = new QHBoxLayout( bar ) ;
		layout->setContentsMargins( 18,10,18,10 ) ;
		layout->setSpacing( 14 ) ;

		return bar ;
	}

	void finalizeActionBar( QFrame * bar )
	{
		if( !bar ){
			return ;
		}

		bar->adjustSize() ;
		bar->setFixedHeight( bar->sizeHint().height() ) ;
	}

	QWidget * createActionCluster( QWidget * parent,std::initializer_list< QPushButton * > buttons )
	{
		auto cluster = new QWidget( parent ) ;
		cluster->setSizePolicy( QSizePolicy::Maximum,QSizePolicy::Preferred ) ;

		auto layout = new QHBoxLayout( cluster ) ;
		layout->setContentsMargins( 0,0,0,0 ) ;
		layout->setSpacing( 20 ) ;

		for( auto it = buttons.begin() ; it != buttons.end() ; ++it ){

			if( !*it ){
				continue ;
			}

			layout->addWidget( *it ) ;

			if( std::next( it ) != buttons.end() ){
				layout->addSpacing( 18 ) ;
			}
		}

		return cluster ;
	}

	void setupBasicDownloaderLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabBasickDownloader ;

		if( !page || page->layout() ){
			return ;
		}

		auto outer = new QVBoxLayout( page ) ;
		outer->setContentsMargins( 12,12,12,12 ) ;
		outer->setSpacing( 10 ) ;

		auto split = new QSplitter( Qt::Vertical,page ) ;
		split->setChildrenCollapsible( false ) ;
		split->addWidget( ui.plainTextEditLogger ) ;
		split->addWidget( ui.bdTableWidgetList ) ;
		split->setStretchFactor( 0,1 ) ;
		split->setStretchFactor( 1,2 ) ;

		outer->addWidget( split,1 ) ;

		auto form = new QWidget( page ) ;
		auto grid = new QGridLayout( form ) ;
		grid->setContentsMargins( 0,0,0,0 ) ;
		grid->setHorizontalSpacing( 10 ) ;
		grid->setVerticalSpacing( 8 ) ;
		grid->setColumnStretch( 1,1 ) ;

		grid->addWidget( ui.label,0,0 ) ;
		grid->addWidget( ui.lineEditURL,0,1 ) ;
		grid->addWidget( ui.pbPasteClipboard,0,2 ) ;
		grid->addWidget( ui.pbBasicDownloaderPlay,0,3 ) ;

		grid->addWidget( ui.label_2,1,0 ) ;
		grid->addWidget( ui.lineEditOptions,1,1 ) ;
		grid->addWidget( ui.pbOptionsHistory,1,2 ) ;
		grid->addWidget( ui.pbOptionsDownloadOptions,1,3 ) ;

		grid->addWidget( ui.labelEngineName,2,0 ) ;
		grid->addWidget( ui.cbEngineType,2,1,1,3 ) ;

		auto bar = createActionBar( page ) ;
		auto actions = qobject_cast< QHBoxLayout * >( bar->layout() ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbCancel } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbDownload,ui.pbList,ui.pbEntries } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbQuit } ) ) ;
		actions->addStretch( 1 ) ;

		styleActionButton( ui.pbCancel ) ;
		styleActionButton( ui.pbDownload ) ;
		styleActionButton( ui.pbList ) ;
		styleActionButton( ui.pbEntries ) ;
		styleActionButton( ui.pbQuit ) ;

		finalizeActionBar( bar ) ;
		grid->addWidget( bar,3,0,1,4 ) ;

		outer->addWidget( form ) ;
	}

	void setupBatchDownloaderLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabBatchDownloader ;

		if( !page || page->layout() ){
			return ;
		}

		auto outer = new QVBoxLayout( page ) ;
		outer->setContentsMargins( 12,12,12,12 ) ;
		outer->setSpacing( 10 ) ;

		outer->addWidget( ui.tableWidgetBD,1 ) ;

		auto form = new QWidget( page ) ;
		auto grid = new QGridLayout( form ) ;
		grid->setContentsMargins( 0,0,0,0 ) ;
		grid->setHorizontalSpacing( 10 ) ;
		grid->setVerticalSpacing( 8 ) ;
		grid->setColumnStretch( 1,1 ) ;

		grid->addWidget( ui.labelBDEnterUrl,0,0 ) ;
		grid->addWidget( ui.lineEditBDUrl,0,1 ) ;
		grid->addWidget( ui.pbBDPasteClipboard,0,2 ) ;
		grid->addWidget( ui.cbBDMonitorClipboardContent,0,3 ) ;

		grid->addWidget( ui.labelBDEnterOptions,1,0 ) ;
		grid->addWidget( ui.lineEditBDUrlOptions,1,1 ) ;
		grid->addWidget( ui.pbBDOptionsHistory,1,2 ) ;
		grid->addWidget( ui.pbBDOptionsDownload,1,3 ) ;

		grid->addWidget( ui.labelBDEngineName,2,0 ) ;
		grid->addWidget( ui.cbEngineTypeBD,2,1,1,3 ) ;

		auto bar = createActionBar( page ) ;
		auto actions = qobject_cast< QHBoxLayout * >( bar->layout() ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbBDCancel } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbBDDownload,ui.pbBDAdd,ui.pbBDOptions } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbBDQuit } ) ) ;
		actions->addStretch( 1 ) ;

		styleActionButton( ui.pbBDCancel ) ;
		styleActionButton( ui.pbBDAdd ) ;
		styleActionButton( ui.pbBDDownload ) ;
		styleActionButton( ui.pbBDQuit ) ;

		finalizeActionBar( bar ) ;
		grid->addWidget( bar,3,0,1,4 ) ;

		outer->addWidget( form ) ;
	}

	void setupPlaylistDownloaderLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabPlayListDownloader ;

		if( !page || page->layout() ){
			return ;
		}

		auto outer = new QVBoxLayout( page ) ;
		outer->setContentsMargins( 12,12,12,12 ) ;
		outer->setSpacing( 10 ) ;

		outer->addWidget( ui.tableWidgetPl,1 ) ;

		auto form = new QWidget( page ) ;
		auto grid = new QGridLayout( form ) ;
		grid->setContentsMargins( 0,0,0,0 ) ;
		grid->setHorizontalSpacing( 10 ) ;
		grid->setVerticalSpacing( 8 ) ;
		grid->setColumnStretch( 1,1 ) ;

		grid->addWidget( ui.labelPLEnterUrl,0,0 ) ;
		grid->addWidget( ui.lineEditPLUrl,0,1 ) ;
		grid->addWidget( ui.pbPLPasteClipboard,0,2 ) ;
		grid->addWidget( ui.pbPlSubscription,0,3 ) ;

		grid->addWidget( ui.labelPLEnterUrlRange,1,0 ) ;
		grid->addWidget( ui.lineEditPLDownloadRange,1,1 ) ;
		grid->addWidget( ui.pbPLRangeHistory,1,2 ) ;
		grid->addWidget( ui.pbClearArchiveFile,1,3 ) ;

		grid->addWidget( ui.labelPLEnterOptions,2,0 ) ;
		grid->addWidget( ui.lineEditPLUrlOptions,2,1 ) ;
		grid->addWidget( ui.pbPLOptionsHistory,2,2 ) ;
		grid->addWidget( ui.pbPLDownloadOptions,2,3 ) ;

		grid->addWidget( ui.labelPLEngineName,3,0 ) ;
		grid->addWidget( ui.cbEngineTypePD,3,1,1,3 ) ;

		auto bar = createActionBar( page ) ;
		auto actions = qobject_cast< QHBoxLayout * >( bar->layout() ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbPLCancel } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbPLDownload,ui.pbPLGetList,ui.pbPLOptions,ui.pbPLChangeTableSize } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbPLQuit } ) ) ;
		actions->addStretch( 1 ) ;

		styleActionButton( ui.pbPLCancel ) ;
		styleActionButton( ui.pbPLDownload ) ;
		styleActionButton( ui.pbPLGetList ) ;
		styleActionButton( ui.pbPLOptions ) ;
		styleActionButton( ui.pbPLChangeTableSize ) ;
		styleActionButton( ui.pbPLQuit ) ;

		finalizeActionBar( bar ) ;
		grid->addWidget( bar,4,0,1,4 ) ;

		outer->addWidget( form ) ;
	}

	void setupLibraryLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabLibrary ;

		if( !page || page->layout() ){
			return ;
		}

		auto outer = new QVBoxLayout( page ) ;
		outer->setContentsMargins( 12,12,12,12 ) ;
		outer->setSpacing( 10 ) ;

		outer->addWidget( ui.tableWidgetLibrary,1 ) ;

		auto bar = createActionBar( page ) ;
		auto actions = qobject_cast< QHBoxLayout * >( bar->layout() ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbLibraryHome,ui.pbLibraryUp,ui.pbLibraryDowloadFolder } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbLibraryCancel } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbLibraryRefresh } ) ) ;
		actions->addSpacing( 28 ) ;
		actions->addWidget( createActionCluster( bar,{ ui.pbLibraryQuit } ) ) ;
		actions->addStretch( 1 ) ;

		styleActionButton( ui.pbLibraryHome ) ;
		styleActionButton( ui.pbLibraryUp ) ;
		styleActionButton( ui.pbLibraryDowloadFolder ) ;
		styleActionButton( ui.pbLibraryRefresh ) ;
		styleActionButton( ui.pbLibraryCancel ) ;
		styleActionButton( ui.pbLibraryQuit ) ;

		finalizeActionBar( bar ) ;
		outer->addWidget( bar ) ;
	}

	void setupConfigurePageLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabConfigure ;

		if( !page || page->layout() ){
			return ;
		}

		auto layout = new QVBoxLayout( page ) ;
		layout->setContentsMargins( 0,0,0,0 ) ;
		layout->setSpacing( 8 ) ;
		layout->addWidget( ui.tabWidgetConfigure,1 ) ;

		auto footer = createActionBar( page ) ;
		auto footerLayout = qobject_cast< QHBoxLayout * >( footer->layout() ) ;
		footerLayout->addWidget( createActionCluster( footer,{ ui.pbConfigureSave } ) ) ;
		footerLayout->addSpacing( 28 ) ;
		footerLayout->addWidget( createActionCluster( footer,{ ui.pbConfigureQuit } ) ) ;
		footerLayout->addStretch( 1 ) ;

		styleActionButton( ui.pbConfigureSave ) ;
		styleActionButton( ui.pbConfigureQuit ) ;

		finalizeActionBar( footer ) ;
		layout->addWidget( footer ) ;
	}

	void setupConfigureEngineOptionsLayout( Ui::MainWindow& ui )
	{
		auto page = ui.EngineDefaultOptions ;

		if( !page || page->layout() ){
			return ;
		}

		auto scrollPage = createScrollPage( page ) ;
		auto outer = scrollPage.layout ;

		auto engineSection = createSection( QObject::tr( "Engine" ),scrollPage.content ) ;
		auto engineGrid = new QGridLayout( engineSection ) ;
		engineGrid->setContentsMargins( 12,12,12,12 ) ;
		engineGrid->setHorizontalSpacing( 10 ) ;
		engineGrid->setVerticalSpacing( 8 ) ;
		engineGrid->setColumnStretch( 1,1 ) ;

		engineGrid->addWidget( ui.labelConfigureEngines,0,0 ) ;
		engineGrid->addWidget( ui.cbConfigureEngines,0,1,1,2 ) ;

		engineGrid->addWidget( ui.labelPathToCookieFile,1,0 ) ;
		engineGrid->addWidget( ui.lineEditConfigureCookieBrowserName,1,1 ) ;
		engineGrid->addWidget( ui.pbConfigureSetPathToCookieFile,1,2 ) ;
		engineGrid->addWidget( ui.cbCookieSource,1,3 ) ;

		engineGrid->addWidget( ui.labelConfigureTextEncoding,2,0 ) ;
		engineGrid->addWidget( ui.lineEditConfigureTextEncoding,2,1 ) ;
		engineGrid->addWidget( ui.cbDenoEnableAutoDownload,2,2,1,2 ) ;

		auto optionsSection = createSection( QObject::tr( "Default Options" ),scrollPage.content ) ;
		auto optionsLayout = new QVBoxLayout( optionsSection ) ;
		optionsLayout->setContentsMargins( 12,12,12,12 ) ;
		optionsLayout->setSpacing( 10 ) ;
		optionsLayout->addWidget( ui.tableWidgetEnginesDefaultOptions,1 ) ;

		auto addRow = new QGridLayout ;
		addRow->setContentsMargins( 0,0,0,0 ) ;
		addRow->setHorizontalSpacing( 10 ) ;
		addRow->setVerticalSpacing( 8 ) ;
		addRow->setColumnStretch( 1,1 ) ;

		addRow->addWidget( ui.labelConfigureOptionsToAdd,0,0 ) ;
		addRow->addWidget( ui.lineEditAddDefaultDownloadOption,0,1 ) ;
		addRow->addWidget( ui.pbConfigureEngineDefaultOptions,0,2 ) ;
		addRow->addWidget( ui.pbAddDefaultDownloadOption,0,3 ) ;
		optionsLayout->addLayout( addRow ) ;

		auto editPanel = new QFrame( scrollPage.content ) ;
		editPanel->setObjectName( "configureEditPanel" ) ;
		auto editPanelLayout = new QVBoxLayout( editPanel ) ;
		editPanelLayout->setContentsMargins( 12,12,12,12 ) ;
		editPanelLayout->setSpacing( 10 ) ;
		editPanelLayout->addWidget( ui.labelEditConfigOptions ) ;
		editPanelLayout->addWidget( ui.textEditConfigureEditOption,1 ) ;

		auto editActions = new QHBoxLayout ;
		editActions->setContentsMargins( 0,0,0,0 ) ;
		editActions->setSpacing( 10 ) ;
		editActions->addStretch( 1 ) ;
		editActions->addWidget( ui.pbConfigureSaveEditOption ) ;
		editActions->addWidget( ui.pbConfigureSaveEditOptionCancel ) ;
		editActions->addStretch( 1 ) ;
		editPanelLayout->addLayout( editActions ) ;

		outer->addWidget( engineSection ) ;
		outer->addWidget( optionsSection,1 ) ;
		outer->addWidget( editPanel ) ;
		outer->addStretch( 1 ) ;
	}

	void setupConfigurePresetOptionsLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabPresetOptions ;

		if( !page || page->layout() ){
			return ;
		}

		auto scrollPage = createScrollPage( page ) ;
		auto outer = scrollPage.layout ;

		auto confirmPanel = new QFrame( scrollPage.content ) ;
		confirmPanel->setObjectName( "configureConfirmPanel" ) ;
		auto confirmLayout = new QVBoxLayout( confirmPanel ) ;
		confirmLayout->setContentsMargins( 12,12,12,12 ) ;
		confirmLayout->setSpacing( 10 ) ;
		confirmLayout->addWidget( ui.labelBaseConfirmResetPresetText ) ;

		auto confirmButtons = new QHBoxLayout ;
		confirmButtons->setContentsMargins( 0,0,0,0 ) ;
		confirmButtons->setSpacing( 10 ) ;
		confirmButtons->addStretch( 1 ) ;
		confirmButtons->addWidget( ui.pbConfigureConfirmResetNo ) ;
		confirmButtons->addWidget( ui.pbConfigureConfirmResetYes ) ;
		confirmButtons->addStretch( 1 ) ;
		confirmLayout->addLayout( confirmButtons ) ;

		auto listSection = createSection( QObject::tr( "Preset List" ),scrollPage.content ) ;
		auto listLayout = new QVBoxLayout( listSection ) ;
		listLayout->setContentsMargins( 12,12,12,12 ) ;
		listLayout->setSpacing( 10 ) ;
		listLayout->addWidget( ui.tableWidgetConfigurePresetOptions,1 ) ;

		auto editorSection = createSection( QObject::tr( "Preset Editor" ),scrollPage.content ) ;
		auto editorLayout = new QGridLayout( editorSection ) ;
		editorLayout->setContentsMargins( 12,12,12,12 ) ;
		editorLayout->setHorizontalSpacing( 10 ) ;
		editorLayout->setVerticalSpacing( 8 ) ;
		editorLayout->setColumnStretch( 1,1 ) ;

		editorLayout->addWidget( ui.labelConfugureWebSite,0,0 ) ;
		editorLayout->addWidget( ui.lineEditConfigureWebsite,0,1,1,3 ) ;
		editorLayout->addWidget( ui.labelConfugureUiName,1,0 ) ;
		editorLayout->addWidget( ui.lineEditConfigureUiName,1,1,1,3 ) ;
		editorLayout->addWidget( ui.labelConfigureOptionsPresetOptiions,2,0 ) ;
		editorLayout->addWidget( ui.lineEditConfigurePresetOptions,2,1,1,3 ) ;

		auto editorActions = new QHBoxLayout ;
		editorActions->setContentsMargins( 0,0,0,0 ) ;
		editorActions->setSpacing( 10 ) ;
		editorActions->addWidget( ui.pbConfigureSetPresetDefaults ) ;
		editorActions->addWidget( ui.pbConfigureAddToPresetList ) ;
		editorActions->addStretch( 1 ) ;
		editorLayout->addLayout( editorActions,3,0,1,4 ) ;

		outer->addWidget( confirmPanel ) ;
		outer->addWidget( listSection,1 ) ;
		outer->addWidget( editorSection ) ;
		outer->addStretch( 1 ) ;
	}

	void setupConfigureExtensionsLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabWidgetConfigureExtensions ;

		if( !page || page->layout() ){
			return ;
		}

		auto scrollPage = createScrollPage( page ) ;
		auto outer = scrollPage.layout ;

		auto actionSection = createSection( QObject::tr( "Extension Actions" ),scrollPage.content ) ;
		auto actionLayout = new QGridLayout( actionSection ) ;
		actionLayout->setContentsMargins( 12,12,12,12 ) ;
		actionLayout->setHorizontalSpacing( 10 ) ;
		actionLayout->setVerticalSpacing( 10 ) ;

		actionLayout->addWidget( ui.pbConfigureAddAPlugin,0,0,1,2 ) ;
		actionLayout->addWidget( ui.pbConfigureUpdateExtensions,1,0,1,2 ) ;
		actionLayout->addWidget( ui.pbConfigureRemoveAPlugin,2,0,1,2 ) ;
		actionLayout->addWidget( ui.pbOpenBinFolder,3,0 ) ;
		actionLayout->addWidget( ui.pbOpenExtensionFolder,3,1 ) ;

		outer->addWidget( actionSection ) ;
		outer->addStretch( 1 ) ;
	}

	void setupConfigureUrlManagerLayout( Ui::MainWindow& ui )
	{
		auto page = ui.UrlManager ;

		if( !page || page->layout() ){
			return ;
		}

		auto scrollPage = createScrollPage( page ) ;
		auto outer = scrollPage.layout ;

		auto tableSection = createSection( QObject::tr( "URL Mappings" ),scrollPage.content ) ;
		auto tableLayout = new QVBoxLayout( tableSection ) ;
		tableLayout->setContentsMargins( 12,12,12,12 ) ;
		tableLayout->setSpacing( 10 ) ;
		tableLayout->addWidget( ui.tableWidgetConfigureUrl,1 ) ;

		auto formSection = createSection( QObject::tr( "Add Mapping" ),scrollPage.content ) ;
		auto formLayout = new QGridLayout( formSection ) ;
		formLayout->setContentsMargins( 12,12,12,12 ) ;
		formLayout->setHorizontalSpacing( 10 ) ;
		formLayout->setVerticalSpacing( 8 ) ;
		formLayout->setColumnStretch( 1,1 ) ;

		formLayout->addWidget( ui.labelConfigureEngines_2,0,0 ) ;
		formLayout->addWidget( ui.cbConfigureEnginesUrlManager,0,1,1,3 ) ;
		formLayout->addWidget( ui.label_4,1,0 ) ;
		formLayout->addWidget( ui.lineEditConfigureManageUrl,1,1,1,3 ) ;
		formLayout->addWidget( ui.label_5,2,0 ) ;
		formLayout->addWidget( ui.lineEditConfigureManageOptions,2,1,1,3 ) ;
		formLayout->addWidget( ui.pbConfigureManageUrl,3,0,1,4 ) ;

		outer->addWidget( ui.label_3 ) ;
		outer->addWidget( tableSection,1 ) ;
		outer->addWidget( formSection ) ;
		outer->addStretch( 1 ) ;
	}

	void setupConfigureProxyLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabProxySettings ;

		if( !page || page->layout() ){
			return ;
		}

		auto scrollPage = createScrollPage( page ) ;
		auto outer = scrollPage.layout ;

		auto proxySection = createSection( QObject::tr( "Proxy Mode" ),scrollPage.content ) ;
		auto proxyLayout = new QVBoxLayout( proxySection ) ;
		proxyLayout->setContentsMargins( 12,12,12,12 ) ;
		proxyLayout->setSpacing( 10 ) ;

		proxyLayout->addWidget( ui.rbNoProxy ) ;
		proxyLayout->addWidget( ui.rbUseSystemProxy ) ;
		proxyLayout->addWidget( ui.rbGetFromEnv ) ;
		proxyLayout->addWidget( ui.rbUseManualProxy ) ;
		proxyLayout->addWidget( ui.labelProxy ) ;
		proxyLayout->addWidget( ui.lineEditCustormProxyAddress ) ;

		outer->addWidget( proxySection ) ;
		outer->addStretch( 1 ) ;
	}

	void setupConfigureUiScaleLayout( Ui::MainWindow& ui )
	{
		auto page = ui.tabUiScale ;

		if( !page || page->layout() ){
			return ;
		}

		auto scrollPage = createScrollPage( page ) ;
		auto outer = scrollPage.layout ;

		auto scaleSection = createSection( QObject::tr( "UI Scale" ),scrollPage.content ) ;
		auto scaleLayout = new QVBoxLayout( scaleSection ) ;
		scaleLayout->setContentsMargins( 12,12,12,12 ) ;
		scaleLayout->setSpacing( 12 ) ;
		scaleLayout->addWidget( ui.labelUIScale ) ;
		scaleLayout->addWidget( ui.labelUIScaleCurrentValue ) ;

		auto scaleButtons = new QHBoxLayout ;
		scaleButtons->setContentsMargins( 0,0,0,0 ) ;
		scaleButtons->setSpacing( 10 ) ;
		scaleButtons->addWidget( ui.pbConfigureScaleDown ) ;
		scaleButtons->addWidget( ui.pbConfigureScaleReset ) ;
		scaleButtons->addWidget( ui.pbConfigureScaleUp ) ;
		scaleLayout->addLayout( scaleButtons ) ;

		outer->addWidget( scaleSection ) ;
		outer->addStretch( 1 ) ;
	}
}

MainWindow::MainWindow( QApplication& app,
			settings& s,
			translator& t,
			const engines::enginePaths& paths,
			const utility::cliArguments& args ) :
	m_trayIcon( s.getIcon( "media-downloader" ) ),
	m_qApp( app ),
	m_appName( "Media Downloader" ),
	m_ui( this ),
	m_logger( m_ui.plainTextEditLogger(),this,s ),
	m_engines( m_logger,paths,s,utility::sequentialID() ),
	m_printOutPut( args ),
	m_tabManager( s,t,m_engines,m_logger,m_ui.get(),*this,*this,m_appName,m_printOutPut ),
	m_settings( s ),
	m_showTrayIcon( s.showTrayIcon() ),
	m_shortcut( this )
{
MainWindow::setUpSignals( this ) ;

	this->setTitle( m_appName ) ;

	qRegisterMetaType< utility::networkReply >() ;
	qRegisterMetaType< reportFinished >() ;

	wrapTabWidgetPages( m_ui.get().tabWidget ) ;

	setupBasicDownloaderLayout( m_ui.get() ) ;
	setupBatchDownloaderLayout( m_ui.get() ) ;
	setupPlaylistDownloaderLayout( m_ui.get() ) ;
	setupLibraryLayout( m_ui.get() ) ;
	setupConfigurePageLayout( m_ui.get() ) ;
	setupConfigureGeneralOptionsLayout( m_ui.get() ) ;
	setupConfigureEngineOptionsLayout( m_ui.get() ) ;
	setupConfigurePresetOptionsLayout( m_ui.get() ) ;
	setupConfigureExtensionsLayout( m_ui.get() ) ;
	setupConfigureUrlManagerLayout( m_ui.get() ) ;
	setupConfigureProxyLayout( m_ui.get() ) ;
	setupConfigureUiScaleLayout( m_ui.get() ) ;

	m_ui.get().tabWidget->setDocumentMode( true ) ;
	m_ui.get().tabWidget->setUsesScrollButtons( true ) ;
	m_ui.get().tabWidgetConfigure->setDocumentMode( true ) ;
	m_ui.get().tabWidgetConfigure->setUsesScrollButtons( true ) ;

	m_settings.setMainWindowDimensions( this ) ;

	this->window()->setWindowIcon( m_trayIcon.icon() ) ;

	m_trayIcon.setContextMenu( [ this,&t ](){

		auto m = new QMenu( this ) ;

		auto ac = t.addAction( m,{ tr( "Quit" ),"Quit","Quit" },true ) ;

		connect( ac,&QAction::triggered,[ this ](){

			this->quitApp() ;
		} ) ;

		return m ;
	}() ) ;

	auto qe = Qt::QueuedConnection ;

	connect( this,&MainWindow::processEventSignal,this,&MainWindow::processEventSlot,qe ) ;

	connect( &m_trayIcon,&QSystemTrayIcon::activated,[ this ]( QSystemTrayIcon::ActivationReason ){

		if( this->isVisible() ){

			this->hide() ;
		}else{
			this->show() ;
		}
	} ) ;

	if( m_showTrayIcon ){

		if( QSystemTrayIcon::isSystemTrayAvailable() ){

			m_trayIcon.show() ;
		}else{
			util::Timer( 1000,[ this ]( int counter ){

				if( QSystemTrayIcon::isSystemTrayAvailable() ){

					m_trayIcon.show() ;

					return true ;
				}else{
					if( counter == 5 ){

						/*
						 * We have waited for system tray to become
						 * available and we can wait no longer, display
						 * it and hope for the best.
						 */
						m_trayIcon.show() ;

						return true ;
					}else{
						return false ;
					}
				}
			} ) ;
		}

		m_trayIcon.show() ;
	}

	paths.confirmPaths( m_logger ) ;
}

void MainWindow::showTrayIcon( bool e )
{
	m_showTrayIcon = e ;

	if( e ){

		m_trayIcon.show() ;
	}else{
		m_trayIcon.hide() ;
	}
}

void MainWindow::keyPressEvent( QKeyEvent * e )
{
	auto key = static_cast< Qt::Key >( e->key() ) ;

	if( e->modifiers() & Qt::CTRL ){

		if( key == Qt::Key_D ){

			m_tabManager.keyPressed( utility::mainWindowKeyCombo::CTRL_D ) ;

		}else if( key == Qt::Key_A ){

			m_tabManager.keyPressed( utility::mainWindowKeyCombo::CTRL_A ) ;
		}
	}else{
		if( key == Qt::Key_Enter || key == Qt::Key_Return ){

			m_tabManager.keyPressed( utility::mainWindowKeyCombo::ENTER ) ;
		}
	}

	QWidget::keyPressEvent( e ) ;
}

void MainWindow::retranslateUi()
{
	m_ui.retranslateUi( this ) ;
}

void MainWindow::setTitle( const QString& m )
{
	if( m.isEmpty() ){

		this->resetTitle() ;
	}else{
		this->window()->setWindowTitle( m ) ;
	}
}

void MainWindow::resetTitle()
{
	this->setTitle( m_appName ) ;
}

void MainWindow::Show()
{
	this->show() ;
	this->raise() ;
	this->activateWindow() ;
}

void MainWindow::processEvent( const QByteArray& m )
{
	emit this->processEventSignal( m ) ;
}

void MainWindow::processEventSlot( const QByteArray& e )
{
	m_tabManager.gotEvent( e ) ;
}

void MainWindow::quitApp()
{
	m_settings.setTabNumber( m_ui.currentIndex() ) ;
	m_settings.saveMainWindowState( this ) ;

	m_tabManager.exiting() ;

	if( m_dataNotSaved ){

		m_dataNotSaved = false ;
		this->saveData() ;
	}

	QCoreApplication::quit() ;
}

void MainWindow::saveData()
{
	m_tabManager.batchDownloader().saveData() ;
	m_tabManager.playlistDownloader().saveData() ;
}

void MainWindow::notifyOnDownloadComplete( const QString& e )
{
	auto m = QSystemTrayIcon::Information ;
	auto s = m_settings.desktopNotificationTimeOut() ;

	m_trayIcon.showMessage( "Download Complete",e,m,s ) ;
}

void MainWindow::notifyOnAllDownloadComplete( const QString& e )
{
	auto m = QSystemTrayIcon::Information ;
	auto s = m_settings.desktopNotificationTimeOut() ;

	m_trayIcon.showMessage( "All Downloads Complete",e,m,s ) ;
}

MainWindow::~MainWindow()
{
	m_settings.saveMainWindowState( this ) ;
}

MainWindow * MainWindow::m_mainWindow ;

void MainWindow::setUpSignals( MainWindow * m )
{
	m_mainWindow = m ;
	MainWindow::setUpSignal( SIGTERM,SIGSEGV,SIGINT,SIGABRT ) ;
}

void MainWindow::signalHandler( int )
{
	m_mainWindow->quitApp() ;
}

void MainWindow::setUpSignal( int sig )
{
	std::signal( sig,MainWindow::signalHandler ) ;
}

void MainWindow::closeEvent( QCloseEvent * )
{
	if( m_showTrayIcon ){

		this->hide() ;
	}else{
		this->quitApp() ;
	}
}
