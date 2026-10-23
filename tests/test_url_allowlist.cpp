/*
 * Testes de utils::url::isAllowed (src/utils/url_allowlist.hpp).
 */

#include "utils/url_allowlist.hpp"

#include <cstdio>

static int failures = 0 ;

static void check( const char * url,bool expected )
{
	QStringList allowed{ "https://api.github.com",
			     "https://bellard.org/quickjs/binary_releases",
			     "https://codeberg.org" } ;

	auto m = utils::url::isAllowed( QString::fromUtf8( url ),allowed ) ;

	if( m != expected ){

		failures++ ;
		std::printf( "FAIL: %s -> %s, esperado %s\n",url,m ? "true" : "false",expected ? "true" : "false" ) ;
	}
}

int main()
{
	check( "https://api.github.com/repos/yt-dlp/yt-dlp/releases/latest",true ) ;
	check( "https://API.GITHUB.COM/repos/x/y/releases",true ) ;
	check( "https://api.github.com:443/repos/x/y",true ) ;
	check( "https://codeberg.org/api/v1/repos/mikf/gallery-dl/releases",true ) ;
	check( "https://bellard.org/quickjs/binary_releases/LATEST.json",true ) ;
	check( "https://bellard.org/quickjs/binary_releases",true ) ;

	check( "http://api.github.com/repos/x/y",false ) ;
	check( "https://api.github.com.evil.tld/repos/x/y",false ) ;
	check( "https://api.github.com@evil.tld/repos/x/y",false ) ;
	check( "https://user:pass@api.github.com/repos/x/y",false ) ;
	check( "https://api.github.com:8443/repos/x/y",false ) ;
	check( "https://bellard.org/quickjs/binary_releases_evil/x.zip",false ) ;
	check( "https://bellard.org/quickjs/binary_releases/../../other",false ) ;
	check( "https://bellard.org/other",false ) ;
	check( "https://evil.tld/api.github.com",false ) ;
	check( "",false ) ;
	check( "not a url",false ) ;

	if( failures ){

		std::printf( "%d falha(s)\n",failures ) ;
		return 1 ;
	}

	std::printf( "OK\n" ) ;
	return 0 ;
}
