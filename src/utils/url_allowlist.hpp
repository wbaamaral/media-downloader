/*
 *
 *  Copyright (c) 2026
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

#pragma once

#include <QString>
#include <QStringList>
#include <QUrl>

namespace utils
{
	namespace url
	{
		/*
		 * Returns true if "url" is an https url whose host is exactly the host of
		 * one of the "allowed" entries and whose path is equal to, or a sub path of,
		 * that entry's path.
		 *
		 * A plain prefix comparison is not enough because "https://api.github.com.evil.tld"
		 * and "https://api.github.com@evil.tld" both start with "https://api.github.com".
		 */
		static inline bool isAllowed( const QString& url,const QStringList& allowed )
		{
			QUrl u( url,QUrl::StrictMode ) ;

			if( !u.isValid() || u.scheme() != "https" || u.host().isEmpty() ){

				return false ;
			}

			if( !u.userInfo().isEmpty() || ( u.port() != -1 && u.port() != 443 ) ){

				return false ;
			}

			auto host = u.host().toLower() ;
			auto path = u.path() ;

			if( path.contains( "/../" ) || path.endsWith( "/.." ) ){

				return false ;
			}

			for( const auto& it : allowed ){

				QUrl a( it ) ;

				if( a.host().toLower() != host ){

					continue ;
				}

				auto prefix = a.path() ;

				while( prefix.endsWith( '/' ) ){

					prefix.chop( 1 ) ;
				}

				if( prefix.isEmpty() || path == prefix || path.startsWith( prefix + "/" ) ){

					return true ;
				}
			}

			return false ;
		}

		/*
		 * Trusted origins for binary engine and application update downloads.
		 */
		static inline const QStringList& binaryDownloadAllowlist()
		{
			static const QStringList list{
				"https://github.com",
				"https://objects.githubusercontent.com",
				"https://raw.githubusercontent.com",
				"https://codeberg.org",
				"https://bellard.org/quickjs/binary_releases"
			} ;

			return list ;
		}

		/*
		 * Returns true if "url" is a trusted download URL for engine or application assets.
		 */
		static inline bool isBinaryDownloadAllowed( const QString& url )
		{
			return isAllowed( url,binaryDownloadAllowlist() ) ;
		}
	}
}
