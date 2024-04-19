/*****************************************************************************
 * File: du2c
 *
 * Purpose: change a DOS format (CR LF) or a UNIX format (LF) file to a CP/M
 *          format ending with ^Z (EOF)
 *          We minimise the include files, so as to make this portable to run
 *          on CP/M as well as posixish, and we use raw i/o rather than buffered
 *          Uaing the CP/M BDSC tools, we use the provided chario source to
 *          achieve unbuffered raw character i/o
 *
 * Build:        For debugging with cygwin:
 *          gcc -g -Wall du2c.c -o du2c
 *               and to stepwise debug
 *          gdb du2c.exe
 *
 *               or on CP/M BDSC compiler
 *          cc du2c -o
 *          cc chario -o
 *          clink du2c chario -s -n -w -e <ldata>   e.g. ldata=2400
 *
 *               and to interactively ebug on CP/M
 *          a:zsid du2c.com du2c.sym
 *
 * Version: 2024.apr.15 jhr, started, running on cygwin in a few hours
 *          2024.apr.19 jhr, got working on Agon CP/M
 *                            K&R-ness (pre-processor, keywords, O/S support,..)
 *                            plus sources need to be EOF-terminated, hence
 *                             this program...
 *                            CP/M is set up for write() sector-sized chunks
 *                             so we use chario supplied with BDSC
 *****************************************************************************/

/*** DEFINITIONS ***/
#define MAX_FILE_NAME_LEN     80
#define CPM_FILE_SECTOR_SIZE  128
#define EOF_CHAR              26    /* ctrl-Z proper CP/M EOF */
#define NULL_CHAR             0     /* often marks EOF actual */
#define CRET_CHAR             13
#define LF_CHAR               10
#ifdef __CYGWIN__
#define STATIC	static
#define CONST	const
#else
#define STATIC
#define CONST
#define PROGNAME              "du2c"
#endif
 

/*** INCLUDES ***/
#include <stdio.h>
#ifdef __CYGWIN__
#   include <fcntl.h>  /* O_CREAT, O_RDONLY, O_TRUNC */
#else
    int  creat( CONST char *, mode_t );
    int  open( CONST char *, int, ... );
#define INPUT           0
#define OUTPUT          1
#define UPDATE          2
#define O_RDONLY     INPUT
#endif

/* in lieu of <unistd.h> */
int close( int CONST );
int read( CONST int , void *, size_t const );
int write( CONST int , CONST void *, size_t CONST );

/* in lieu of stdbool.h */
#define _Bool int
#define true  (1)
#define false (0)


/*** TYPES ***/
    /* no enums in K&R, so we fall back to definitions */
#define res_OK		( 0 ) 
#define res_ARGS	( res_OK + 1 )
#define res_UNKNOWN_ARG	( res_ARGS + 1 )
#define res_READ_FAIL	( res_UNKNOWN_ARG + 1 )
#define res_WRITE_FAIL	( res_READ_FAIL + 1 )
#define res_CREAT_FAIL	( res_WRITE_FAIL + 1 )

#define RESULT	int

#ifndef __CYGWIN__
    /* i/o buffer (from bdsc/chario.c) */
    struct iobuf 
    {
        int fd;
        int isect;			/* currently buffered sector */
        int nextc;			/* index of next char in buffer */
        char buff [128];
    };
#endif


/*** GLOBAL VARIABLES ***/
STATIC _Bool infileset;
STATIC char infile[ MAX_FILE_NAME_LEN ];
STATIC _Bool outfileset;
STATIC char outfile[ MAX_FILE_NAME_LEN ];
STATIC _Bool verbose;


/*** GLOBAL FUNCTIONS ***/
STATIC RESULT getargs( int CONST, char CONST * CONST * );
STATIC void usage( char CONST * );
STATIC int lstrncpy( char * CONST, char CONST *, int CONST );
STATIC RESULT genout( void );
#ifndef __CYGWIN__
    STATIC int lmemset( char * CONST, int CONST, int CONST );

    /* i/o buffer (from bdsc/chario.c) */
    int ccreat( struct iobuf*, char* );
    int cread( struct iobuf*, char*, unsigned int );
    int cwrite( struct iobuf*, char*, unsigned int );
    int cclose( struct iobuf* );
#endif

STATIC RESULT getargs( argc, argv )
    int CONST argc;
    char CONST * CONST * argv;
{
    RESULT r;
    int infilearg;
    int outfilearg;
    int i;

    r = res_OK;
    infilearg = 0;
    outfilearg = 0;
    
    for( i = 0;( argc > i )&&( res_OK == r ); )
    {
        int idx;  /* per argument index */

        idx = 0;
        if( 0 == i )
        {
            /* skip over program name.
             * See BDS C User's Guide, Leor Zolman, ch:1.10 Operational Caveats
             * 2. argv and argv passed into main [by the BDSC startup] will
             * never include an entry for the command name itself argv[0]
             * because CP/M does not make that information available to
             * transient programs on start-up. */
            i++;
            continue;
        }

        if( '-' == argv[ i ][ 0 ])
        {
            idx = 1;
            if( '-' == argv[ i ][ idx ])
            {
                idx++;  /* skip over second '-'; arguments may be -a or --a */
            }

            switch( argv[ i ][ idx ])
            {
                case 'i':   /* input file name */
                case 'I':     /* CP/M capitailises all CCP input */
                {
                    infilearg = true;
                    i++;
                    continue;
                }
                break;

                case 'o':   /* output file name */
                case 'O':     /* CP/M capitailises all CCP input */
                {
                    outfilearg = true;
                    i++;
                    continue;
                }
                break;

                case 'v' :  /* verbose output */
                case 'V':     /* CP/M capitailises all CCP input */
                {
                    verbose = true;
                    i++;
                    continue;
                }
                break;

                default:
                {
                    i++;
                    r = res_UNKNOWN_ARG;
                    continue;
                }
                break;
            }
        }
        else
        {
            if(( true == infilearg )||
               (( false == outfilearg )&&( false == infileset )))
            {
                /* first non'-' argument assume the input file name */
                lstrncpy( infile, argv[ i ], MAX_FILE_NAME_LEN - 1 );
                infile[ MAX_FILE_NAME_LEN - 1 ]= 0;
                infileset = true;
                infilearg = false;
                i++;
            }
            else
            if(( true == outfilearg )||( false == outfileset ))
            {
                /* second non'-' argument assume the output file name */
                lstrncpy( outfile, argv[ i ], MAX_FILE_NAME_LEN - 1 );
                outfile[ MAX_FILE_NAME_LEN - 1 ]= 0;
                outfileset = true;
                outfilearg = false;
                i++;
            }
        }
    }

    if( !( infileset && outfileset ))
    {
        r = res_ARGS;
    }

    return( r );
}


STATIC void usage( pname )
    char CONST * pname;
{
    printf( "Usage: %s <in file> <out file>\n", pname );
}


STATIC int lstrncpy( dst, src, maxlen )
    char * CONST dst;
    char CONST * src;
    int CONST maxlen;
{
    int i;
    char *d;
    char CONST * s;

    i = 0;
    d = dst;
    s = src;
    
    for( ; *d++ = *s++, maxlen > i++; )
      ;
    d[ i ]= 0;

    return( i );
}

#ifndef __CYGWIN__
STATIC int lmemset( p, val, nBytes )
    char * CONST p;
    int CONST val;
    int CONST nBytes;
{
    int i;

    for( i = 0; nBytes > i; i++ )
    {
    	p[ i ]= val;
    }
    
    return( nBytes );
} 	    
#endif

#ifdef __CYGWIN__
STATIC RESULT genout( void )
{
    RESULT r = res_OK;
    int fdi;    /* input file handle */
    int fdo;    /* output file handle */
    int len = 0;

    if( verbose )
    {
        printf( "Copying from input file '%s'\n", infile );
        printf( "To output file '%s'\n", outfile );
    }

    fdi = open( infile, O_RDONLY );
    if( 0 < fdi )
    {
        fdo = creat( outfile, 0755 /* S_IRWXU  | S_IRGRP | S_IROTH  */ );
        if( 0 < fdo )
        {
            char ch;
            int n;

            /* copy the input to the output as simply as possible,
            without assuming any underlying O/S buffering;
            this will be slow and make O/S calls for each byte */
            for( ;; )
            {
                n = read( fdi, &ch, sizeof( ch ));
                if( 0 < n )
                {
                    n = write( fdo, &ch, sizeof( ch ));
                    if( 0 < n )
                    {
                        len++;
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }

            /* add a trailing ^Z character */
            ch = 26; 
            write( fdo, &ch, sizeof( ch ));

            close( fdo );

            if( verbose )
            {
                printf( "Copied %d bytes\n", len );
            }
        }
        else
        {
            r = res_WRITE_FAIL;
            if( verbose )
            {
                printf( "Failed to open output file\n" );
            }
        }

        close( fdi );
    }
    else
    {
        r = res_READ_FAIL;
        if( verbose )
        {
            printf( "Failed to open input file\n" );
        }
    }

    return( r );
}
#else /* ! __CYGWIN */
STATIC RESULT genout( )
{
    RESULT r;
    struct iobuf fdi;    /* input file handle */
    struct iobuf fdo;    /* output file handle */
    int len;

    r = res_OK;
    len = 0;
    lmemset( &fdi, 0, sizeof( fdi ));
    lmemset( &fdo, 0, sizeof( fdo ));
        
    if( verbose )
    {
        printf( "Copying from input file '%s'\n", infile );
        printf( "To output file '%s'\n", outfile );
    }

    if( -1 != copen( &fdi, infile ))
    {
        if( -1 != ccreat( &fdo, outfile ))
        {
            char ch;  /* buffer */
            int n;    /* temporary for cread() */
            int m;    /* temporary for cwrite() */
            int f;    /* temporary for cflush() */
            int ff;   /* file flush count */

            /* copy the input to the output as simply as possible,
            without assuming any underlying O/S buffering;
            this will be slow and make O/S calls for each byte */
            ff = 0;
            for( ;; )
            {
                n = cread( &fdi, &ch, sizeof( ch ));
                if( 0 < n )
                {
                    if(( EOF_CHAR == ch )||( NULL_CHAR == ch ))
                    {
                       break;
                    }
                    else
                    {
                        /* Note cwrite doesn't return the number of bytes 
                         * written (or buffered). Instead test for -1 for 
                         * failure */
                        m = cwrite( &fdo, &ch, sizeof( ch ));
                        if( 0 <= m )
                        {
                            len++;
                            ff++;
                            if( 0 ==( ff % CPM_FILE_SECTOR_SIZE ))
                            {
                               f = cflush( &fdo );
                               if( 1 == f )
                               {
                                   ff = 0;
                               }
                               else
                               {
                                   r = res_WRITE_FAIL;
                                   if( verbose )
                                   {
                                       printf( "Failed to flush file %s\n", 
                                             errmsg( errno( )));
                                   }
                                   break;
                               }
                           }
                        }
                        else
                        {
                            r = res_WRITE_FAIL;
                            if( verbose )
                            {
                                printf( "Failed to write file: %s\n", 
                                      errmsg( errno( )));
                            }
                            break;
                        }
                    }
                }
                else
                {
                    r = res_READ_FAIL;
                    if( verbose )
                    {
                        printf( "Failed to read input file: %s\n", 
                              errmsg( errno( )));
                    }
                    break;
                }
            } /* end for */

            if( res_OK == r )
            {
                if( verbose )
                {
                    printf( "Adding EOF marker to output\n" );
                }
                /* add a trailing ^Z character */
                ch = 26; 
                m = cwrite( &fdo, &ch, sizeof( ch ));
                f = cflush( &fdo );
                if( -1 == f )
                {
                    r = res_WRITE_FAIL;
                    if( verbose )
                    {
                        printf( "Failed to flush file %s\n", errmsg( errno( )));
                    }
                }
            }

            cclose( &fdo );
            if( verbose )
            {
                printf( "Copied %d bytes\n", len );
            }
        }
        else
        {
            r = res_CREAT_FAIL;
            if( verbose )
            {
                printf( "Failed to creat output file: %s\n", errmsg( errno( )));
            }
        }

        cclose( &fdi );
    }
    else
    {
        r = res_READ_FAIL;
        if( verbose )
        {
            printf( "Failed to open input file: %s\n", errmsg( errno( )));
        }
    }

    return( r );
}
#endif


/*** PROGRAM ***/
int main( argc, argv )
    int CONST argc;
    char CONST * CONST * argv;
{
    int ret;

    /* initialise globals */
    infileset = false;
    outfileset = false;
    verbose = false;
    
    ret = 0;

    if( res_OK == getargs( argc, argv ))
    {
        if( res_OK != genout( ))
        {
            ret = -1;
        }
    }
    else
    {
#ifdef __CYGWIN__
        usage( argv[ 0 ]);
#else
        usage( PROGNAME );
#endif
    }
    return( ret );
}

