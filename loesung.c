#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef LF
#define LF 10
#endif
#ifndef CR
#define CR 13
#endif

/*=====================================Globals=================================*/
int Return;

/*=====================================Types===================================*/
/* This is supposed to live in a seperate file */
typedef struct Bool
{
    unsigned int V: 1;
} Bool;

typedef struct StringBuffer
{
	char* string;
	unsigned long length;
} StringBuffer;
/*====================================PatricaTrie Defs========================*/
typedef struct PNode {
	struct PNode* parent;
	struct PNode** children;
	unsigned short sizeOfChildren;
	char* value;
	char* key;
} PNode;
/*===================================Utils===================================*/
/**
 * Prints a error-message to stderr and quits the programm
 * @param Message | const char* | the message
 */
void errorAndOut( const char* Message )
{
    fprintf( stderr, "%s\n", Message);
    exit( 23 );
}
/**
 * Returns a substring of given String
 * @param Source | char* | the input string
 * @param From | int | the startposition of the subset
 * @param Length | int | Length of the subset
 * @return | char* | the computed subset
 */
char* substring( char* Source, int From, int Length, Bool* error )
{
    char* Return;
    int Index1, Index2;

    if( NULL == Source )
    {
        return NULL;
    }

    if(-1 == Length)
    {
        Length = strlen( Source );
    }

    if( 0 >= Length )
    {
        return NULL;
    }


    if(0 == From && strlen( Source ) == Length)
    {

        Return = (char *) malloc( ( strlen( Source )+1 )*sizeof(char) );
        if( NULL == Return )
        {
        	error->V = (unsigned int)TRUE;
			return NULL;
		}

        strcpy( Return , Source );
        return Return;
    }

    Return = (char *) malloc( ( Length + 1 )*sizeof(char) );
    if( NULL == Return )
    {
    	error->V = (unsigned int)TRUE;
		return NULL;
	}

    for( Index1=From, Index2=0; Length>Index2; Index1++, Index2++)
    {
        Return[ Index2 ] = Source[ Index1 ];
    }

    Return[ Index2 ] = '\0';
    return Return;
}
/*===================================Flow=====================================*/
/*----------------------------------DICT--------------------------------------*/
void readInputFile( char* Path );
int nextDict( FILE* Source );
void parseDict( FILE* Source );
Bool buildDict( const StringBuffer* Key, const StringBuffer* Value );

int main( int ArgC, char* Arguments[] ) 
{
	Return = 0;
	
	if ( 1 == ArgC ) 
	{
		errorAndOut( "Too few arguments provided." );
	}

	if( 2 < ArgC )
	{
		errorAndOut( "To many arguments provided." );
	}

#ifdef DEBUGPR
	printf( "Start preproc" );
#endif
	readInputFile( Arguments[ 1 ] );
	return 0;
}

/*----------------------------------DICT--------------------------------------*/
void readInputFile( char* Path )
{
	FILE* FilePointer = fopen( Path, "r" );
	char ErrorMsg[ 120 ];
	
	if( !FilePointer || ferror( FilePointer ) )
	{
		snprintf( ErrorMsg, 120, "Cannot open given file (%s).", Path );
		errorAndOut( ErrorMsg );
    }

	parseDict( FilePointer );

	fclose( FilePointer );
}

int nextDict( FILE* Source )
{
	return fgetc( Source );
}

void parseDict( FILE* Source )
{
	unsigned long Pos;
	Bool Mode;
	int CurrentChar;
	int LookAHead;
	StringBuffer Key;
	StringBuffer Value;
	char* Tmp;
	
	Mode.V = 0;
	Pos = 0;

	Key.string = (char*) calloc( sizeof(char), 1 );
	if( NULL == Key.string )
	{
		fclose( Source );
		errorAndOut( strerror( errno ) );
	}

	Value.string = (char*) calloc( sizeof(char), 1 );
	if( NULL == Key.string )
	{
		fclose( Source );
		free( Key.string );
		errorAndOut( strerror( errno ) );
	}

	Key.string[ 0 ] = '\0';
	Value.string[ 0 ] = '\0';
	Key.length = 0;
	Value.length = 0;

	LookAHead = nextDict( Source );

	while( EOF != LookAHead )
	{
		CurrentChar = LookAHead;
		LookAHead = nextDict( Source );
		if( ferror( Source ) )
		{
			fclose( Source );
			free( Key.string );
			free( Value.string );
			errorAndOut( "I/O error when reading." );
		}

		Pos++;
		
		/* CR -> LF && CR LF -> LF: Der Einfachheit halber dürfen Sie neben einem einzelnen Linefeed (LF)
		 * auch ein carriage return(CR) oder ein „CR LF“ als Zeilenumbruch im Wörterbuch akzeptieren.
		 */
		if( CR == CurrentChar )
		{
			if( LF == LookAHead )
			{
				continue;
			}

			CurrentChar = LF;
		}

		/* Colon Rule
		 * Eine Zeile enthält nicht genau einen Doppelpunkt.
		 * Der Doppelpunkt in einer Zeile ist das erste oder das letzte Zeichen
		 */
		if( ':' == CurrentChar )
		{
			if( 
				1 == Mode.V
			||
				LF == LookAHead
			||
				CR == LookAHead
			||
				EOF == LookAHead
			||
				1 == Pos
			)
			{
				free( Key.string );
				free( Value.string );
				fclose( Source );
				errorAndOut( "The given dictionary is incorrectly formed." );
			}

			Mode.V = 1;
			continue;
		}

		/* LineFeed Rule 
		 * Eine Zeile enthält nicht genau einen Doppelpunkt
		 */
		if( LF == CurrentChar )
		{
			if( 0 == Mode.V )
			{
				fclose( Source );
				free( Key.string );
				free( Value.string );
				errorAndOut( "The given dictionary is incorrectly formed." );
			}

			buildDict( &Key, &Value );

			free( Key.string );
			free( Value.string );

			Pos = 0;
			Mode.V = 0;

			Key.string = (char*) calloc( sizeof(char), 1 );
			if( NULL == Key.string )
			{
				fclose( Source );
				errorAndOut( strerror( errno ) );
			}
			
			Value.string = (char*) calloc( sizeof(char), 1 );
			if( NULL == Key.string )
			{
				fclose( Source );
				free( Key.string );
				errorAndOut( strerror( errno ) );
			}
			
			Key.string[ 0 ] = '\0';
			Value.string[ 0 ] = '\0';
			Key.length = 0;
			Value.length = 0;
			continue;
		}

		/* Valid Chars
		 * Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf.
		 */
		if( 'a' > CurrentChar || 'z' < CurrentChar )
		{
			fclose( Source );
			free( Key.string );
			free( Value.string );
			errorAndOut( "The given dictionary is incorrectly formed." );
		}

		/*
		 * a bit slow, could be improved by using buffers
		 */
		if( 0 == Mode.V )
		{
			Tmp = (char*) realloc( Key.string, sizeof( char )*( Key.length + 2 ) ); // Null byte + new Char
			if( NULL == Tmp )
			{
				fclose( Source );
				free( Key.string );
				free( Value.string );
				errorAndOut( strerror( errno ) );
			}

			Key.string = Tmp;
			Key.string[ Key.length ] = (char) CurrentChar;
			Key.length++;
			Key.string[ Key.length ] = '\0';
		}
		else
		{
			Tmp = (char*) realloc( Value.string, sizeof( char )*( Value.length + 2 ) );
			if( NULL == Tmp )
			{
				fclose( Source );
				free( Key.string );
				free( Value.string );
				errorAndOut( strerror( errno ) );
			}

			Value.string = Tmp;
			Value.string[ Value.length ] = (char) CurrentChar;
			Value.length++;
			Value.string[ Value.length ] = '\0';
		}
	}

	/* Rule: Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf. */
	free( Key.string );
	free( Value.string );

	/* Rule: Ein anderes Zeichen als ein Kleinbuchstabe, Doppelpunkt oder Linefeed tritt auf. */
	if( 0 != Pos )
	{
		fclose( Source );
		errorAndOut( "The given dictionary is incorrectly formed - Missing linefeed before end of file." );
	}
}

Bool buildDict( const StringBuffer* Key, const StringBuffer* Value )
{
	Bool Return;
	Return.V = (unsigned int)TRUE;
	printf( "%s:%s\n", Key->string, Value->string );
	return Return;
}
