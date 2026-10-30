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

#include <csignal>

#include <QScreen>
#include <QGuiApplication>
#include <QDebug>

#include <cstring>

#ifndef Q_OS_WIN
	#include <QSocketNotifier>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <signal.h>
	#include <cerrno>

	static int _signalFds[ 2 ] = { -1,-1 } ;
#endif

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

	m_settings.setMainWindowDimensions( this->window() ) ;

	if( this->window()->pos() == QPoint( 0,0 ) ){

		auto screen = QGuiApplication::primaryScreen()->availableGeometry() ;

		int x = ( screen.width() - this->window()->width() ) / 2 ;
		int y = ( screen.height() - this->window()->height() ) / 2 ;

		this->window()->move( x,y ) ;
	}

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
}

void MainWindow::processEvent( const QByteArray& m )
{
	emit this->processEventSignal( m ) ;
}

void MainWindow::processEventSlot( const QByteArray& e )
{
	if( this->isMinimized() ){

		this->showNormal() ;
	}else{
		this->show() ;
	}

	this->raise() ;
	this->activateWindow() ;

	m_tabManager.gotEvent( e ) ;
}

void MainWindow::quitApp()
{
	m_settings.setTabNumber( m_ui.currentIndex() ) ;

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
	m_mainWindow = nullptr ;

	m_settings.saveMainWindowDimensions( this->window()->geometry() ) ;
}

std::atomic< MainWindow * > MainWindow::m_mainWindow{ nullptr } ;

void MainWindow::setUpSignals( MainWindow * m )
{
	m_mainWindow = m ;

	/*
	 * SIGSEGV and SIGABRT are deliberately not handled, there is nothing
	 * safe we can do in them and returning from a SIGSEGV handler just
	 * re-triggers the fault.
	 */
#ifdef Q_OS_WIN
	MainWindow::setUpSignal( SIGTERM,SIGINT ) ;
#else
	if( ::socketpair( AF_UNIX,SOCK_STREAM,0,_signalFds ) != 0 ){

		qDebug() << "Failed to create signal socketpair,signals will not be handled" ;
		return ;
	}

	for( auto fd : _signalFds ){

		::fcntl( fd,F_SETFD,FD_CLOEXEC ) ;
	}

	::fcntl( _signalFds[ 0 ],F_SETFL,::fcntl( _signalFds[ 0 ],F_GETFL ) | O_NONBLOCK ) ;

	auto sn = new QSocketNotifier( _signalFds[ 1 ],QSocketNotifier::Read,m ) ;

	QObject::connect( sn,&QSocketNotifier::activated,[ m,sn ](){

		sn->setEnabled( false ) ;

		char buffer ;

		while( ::read( _signalFds[ 1 ],&buffer,1 ) < 0 && errno == EINTR ){}

		m->quitApp() ;

		sn->setEnabled( true ) ;
	} ) ;

	MainWindow::setUpSignal( SIGTERM,SIGINT ) ;
#endif
}

void MainWindow::signalHandler( int )
{
#ifdef Q_OS_WIN
	/*
	 * On Windows, signal handlers run on a different thread, queue
	 * the call to run on the GUI thread.
	 */
	auto m = m_mainWindow.load() ;

	if( m ){

		QMetaObject::invokeMethod( m,"quitApp",Qt::QueuedConnection ) ;
	}
#else
	/*
	 * Only async-signal-safe functions here, the actual work is done
	 * in the GUI thread when the socket notifier fires.
	 */
	auto err = errno ;

	char m = 1 ;

	auto r = ::write( _signalFds[ 0 ],&m,1 ) ;

	Q_UNUSED( r )

	errno = err ;
#endif
}

void MainWindow::setUpSignal( int sig )
{
#ifdef Q_OS_WIN
	std::signal( sig,MainWindow::signalHandler ) ;
#else
	struct sigaction sa ;

	::memset( &sa,0,sizeof( sa ) ) ;

	sa.sa_handler = MainWindow::signalHandler ;
	sa.sa_flags = SA_RESTART ;

	::sigemptyset( &sa.sa_mask ) ;

	if( ::sigaction( sig,&sa,nullptr ) != 0 ){

		qDebug() << "Failed to set up handler for signal:" << sig ;
	}
#endif
}

bool MainWindow::trayIconUsable()
{
	return m_showTrayIcon && QSystemTrayIcon::isSystemTrayAvailable() && m_trayIcon.isVisible() ;
}

void MainWindow::closeEvent( QCloseEvent * )
{
	if( this->trayIconUsable() ){

		this->hide() ;
	}else{
		this->quitApp() ;
	}
}
