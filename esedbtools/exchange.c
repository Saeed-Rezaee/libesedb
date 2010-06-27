/* 
 * Exchange database export functions
 *
 * Copyright (c) 2010, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <byte_stream.h>
#include <file_stream.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>

/* Define HAVE_LOCAL_LIBUNA for local use of libuna
 */
#if defined( HAVE_LOCAL_LIBUNA )

#include <libuna_base16_stream.h>
#include <libuna_base64_stream.h>
#include <libuna_base64url_stream.h>
#include <libuna_byte_stream.h>
#include <libuna_unicode_character.h>
#include <libuna_url_stream.h>
#include <libuna_utf16_stream.h>
#include <libuna_utf16_string.h>
#include <libuna_utf32_stream.h>
#include <libuna_utf32_string.h>
#include <libuna_utf8_stream.h>
#include <libuna_utf8_string.h>

#elif defined( HAVE_LIBUNA_H )

/* If libtool DLL support is enabled set LIBUNA_DLL_IMPORT
 * before including libuna.h
 */
#if defined( _WIN32 ) && defined( DLL_IMPORT )
#define LIBUNA_DLL_IMPORT
#endif

#include <libuna.h>

#else
#error Missing libuna.h
#endif

/* Define HAVE_LOCAL_LIBFDATETIME for local use of libfdatetime
 */
#if defined( HAVE_LOCAL_LIBFDATETIME )

#include <libfdatetime_date_time_values.h>
#include <libfdatetime_definitions.h>
#include <libfdatetime_error.h>
#include <libfdatetime_fat_date_time.h>
#include <libfdatetime_filetime.h>
#include <libfdatetime_types.h>

#elif defined( HAVE_LIBFDATETIME_H )

/* If libtool DLL support is enabled set LIBFDATETIME_DLL_IMPORT
 * before including libfdatetime.h
 */
#if defined( _WIN32 ) && defined( DLL_IMPORT )
#define LIBFDATETIME_DLL_IMPORT
#endif

#include <libfdatetime.h>

#else
#error Missing libfdatetime.h
#endif

/* Define HAVE_LOCAL_LIBFGUID for local use of libfguid
 */
#if defined( HAVE_LOCAL_LIBFGUID )

#include <libfguid_definitions.h>
#include <libfguid_identifier.h>
#include <libfguid_types.h>

#elif defined( HAVE_LIBFGUID_H )

/* If libtool DLL support is enabled set LIBFGUID_DLL_IMPORT
 * before including libfguid.h
 */
#if defined( _WIN32 ) && defined( DLL_IMPORT )
#define LIBFGUID_DLL_IMPORT
#endif

#include <libfguid.h>

#else
#error Missing libfguid.h
#endif

/* If libtool DLL support is enabled set LIBESEDB_DLL_IMPORT
 * before including libesedb_extern.h
 */
#if defined( _WIN32 ) && defined( DLL_EXPORT )
#define LIBESEDB_DLL_EXPORT
#endif

#include <libesedb.h>

/* Define HAVE_LOCAL_LIBFWNT for local use of libfwnt
 */
#if defined( HAVE_LOCAL_LIBFWNT )

#include <libfwnt_definitions.h>
#include <libfwnt_security_identifier.h>
#include <libfwnt_types.h>

#elif defined( HAVE_LIBFWNT_H )

/* If libtool DLL support is enabled set LIBFWNT_DLL_IMPORT
 * before including libfwnt.h
 */
#if defined( _WIN32 ) && defined( DLL_IMPORT )
#define LIBFWNT_DLL_IMPORT
#endif

#include <libfwnt.h>

#else
#error Missing libfwnt.h
#endif

#include <libsystem.h>

#include "export_handle.h"
#include "exchange.h"

enum EXCHANGE_KNOWN_COLUMN_TYPES
{
	EXCHANGE_KNOWN_COLUMN_TYPE_UNDEFINED,
	EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_32BIT,
	EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_64BIT,
	EXCHANGE_KNOWN_COLUMN_TYPE_FILETIME,
	EXCHANGE_KNOWN_COLUMN_TYPE_GUID,
	EXCHANGE_KNOWN_COLUMN_TYPE_SID,
	EXCHANGE_KNOWN_COLUMN_TYPE_STRING,
};

/* Exports a 32-bit value in a binary data table record value
 * Returns 1 if successful or -1 on error
 */
int exchange_export_record_value_32bit(
     libesedb_record_t *record,
     int record_value_entry,
     uint8_t byte_order,
     FILE *table_file_stream,
     liberror_error_t **error )
{
	uint8_t *value_data    = NULL;
	static char *function  = "exchange_export_record_value_32bit";
	size_t value_data_size = 0;
	uint32_t column_type   = 0;
	uint32_t value_32bit   = 0;
	uint8_t value_flags    = 0;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( ( byte_order != _BYTE_STREAM_ENDIAN_BIG )
	 && ( byte_order != _BYTE_STREAM_ENDIAN_LITTLE ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported byte order: 0x%02" PRIx8 "",
		 function,
		 byte_order );

		return( -1 );
	}
	if( table_file_stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table file stream.",
		 function );

		return( -1 );
	}
	if( libesedb_record_get_column_type(
	     record,
	     record_value_entry,
	     &column_type,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve column type of value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( column_type != LIBESEDB_COLUMN_TYPE_BINARY_DATA )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported column type: %" PRIu32 "",
		 function,
		 column_type );

		return( -1 );
	}
	if( libesedb_record_get_value(
	     record,
	     record_value_entry,
	     &value_data,
	     &value_data_size,
	     &value_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( ( value_flags & ~( LIBESEDB_VALUE_FLAG_VARIABLE_SIZE ) ) == 0 )
	{
		if( value_data != NULL )
		{
			if( value_data_size != 4 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
				 "%s: unsupported value data size: %" PRIzd "",
				 function,
				 value_data_size );

				return( -1 );
			}
			if( byte_order == _BYTE_STREAM_ENDIAN_BIG )
			{
				byte_stream_copy_to_uint32_big_endian(
				 value_data,
				 value_32bit );
			}
			else
			{
				byte_stream_copy_to_uint32_little_endian(
				 value_data,
				 value_32bit );
			}
			fprintf(
			 table_file_stream,
			 "%" PRIu32 "",
			 value_32bit );
		}
	}
	else
	{
		if( value_data != NULL )
		{
			while( value_data_size > 0 )
			{
				fprintf(
				 table_file_stream,
				 "%02" PRIx8 "",
				 *value_data );

				value_data      += 1;
				value_data_size -= 1;
			}
		}
	}
	return( 1 );
}

/* Exports a 64-bit value in a binary data table record value
 * Returns 1 if successful or -1 on error
 */
int exchange_export_record_value_64bit(
     libesedb_record_t *record,
     int record_value_entry,
     uint8_t byte_order,
     FILE *table_file_stream,
     liberror_error_t **error )
{
	uint8_t *value_data    = NULL;
	static char *function  = "exchange_export_record_value_64bit";
	size_t value_data_size = 0;
	uint64_t value_64bit   = 0;
	uint32_t column_type   = 0;
	uint8_t value_flags    = 0;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( ( byte_order != _BYTE_STREAM_ENDIAN_BIG )
	 && ( byte_order != _BYTE_STREAM_ENDIAN_LITTLE ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported byte order: 0x%02" PRIx8 "",
		 function,
		 byte_order );

		return( -1 );
	}
	if( table_file_stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table file stream.",
		 function );

		return( -1 );
	}
	if( libesedb_record_get_column_type(
	     record,
	     record_value_entry,
	     &column_type,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve column type of value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( ( column_type != LIBESEDB_COLUMN_TYPE_BINARY_DATA )
	 && ( column_type != LIBESEDB_COLUMN_TYPE_CURRENCY ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported column type: %" PRIu32 "",
		 function,
		 column_type );

		return( -1 );
	}
	if( libesedb_record_get_value(
	     record,
	     record_value_entry,
	     &value_data,
	     &value_data_size,
	     &value_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( ( value_flags & ~( LIBESEDB_VALUE_FLAG_VARIABLE_SIZE ) ) == 0 )
	{
		if( value_data != NULL )
		{
			if( value_data_size != 8 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
				 "%s: unsupported value data size: %" PRIzd "",
				 function,
				 value_data_size );

				return( -1 );
			}
			if( byte_order == _BYTE_STREAM_ENDIAN_BIG )
			{
				byte_stream_copy_to_uint64_big_endian(
				 value_data,
				 value_64bit );
			}
			else
			{
				byte_stream_copy_to_uint64_little_endian(
				 value_data,
				 value_64bit );
			}
			/* TODO for now print as hexadecimal */
			fprintf(
			 table_file_stream,
			 "0x%" PRIx64 "",
			 value_64bit );
		}
	}
	else
	{
		if( value_data != NULL )
		{
			while( value_data_size > 0 )
			{
				fprintf(
				 table_file_stream,
				 "%02" PRIx8 "",
				 *value_data );

				value_data      += 1;
				value_data_size -= 1;
			}
		}
	}
	return( 1 );
}

/* Exports a filetime value in a binary data table record value
 * Returns 1 if successful or -1 on error
 */
int exchange_export_record_value_filetime(
     libesedb_record_t *record,
     int record_value_entry,
     uint8_t byte_order,
     FILE *table_file_stream,
     liberror_error_t **error )
{
	libcstring_system_character_t filetime_string[ 24 ];

	libfdatetime_filetime_t *filetime = NULL;
	uint8_t *value_data               = NULL;
	static char *function             = "exchange_export_record_value_filetime";
	size_t value_data_size            = 0;
	uint32_t column_type              = 0;
	uint8_t value_flags               = 0;
	int result                        = 0;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( table_file_stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table file stream.",
		 function );

		return( -1 );
	}
	if( libesedb_record_get_column_type(
	     record,
	     record_value_entry,
	     &column_type,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve column type of value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( ( column_type != LIBESEDB_COLUMN_TYPE_BINARY_DATA )
	 && ( column_type != LIBESEDB_COLUMN_TYPE_CURRENCY ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported column type: %" PRIu32 "",
		 function,
		 column_type );

		return( -1 );
	}
	if( libesedb_record_get_value(
	     record,
	     record_value_entry,
	     &value_data,
	     &value_data_size,
	     &value_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( ( value_flags & ~( LIBESEDB_VALUE_FLAG_VARIABLE_SIZE ) ) == 0 )
	{
		if( value_data != NULL )
		{
			if( value_data_size != 8 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
				 "%s: unsupported value data size: %" PRIzd "",
				 function,
				 value_data_size );

				return( -1 );
			}
			if( libfdatetime_filetime_initialize(
			     &filetime,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create filetime.",
				 function );

				return( -1 );
			}
			if( libfdatetime_filetime_copy_from_byte_stream(
			     filetime,
			     value_data,
			     value_data_size,
			     byte_order,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy byte stream to filetime.",
				 function );

				libfdatetime_filetime_free(
				 &filetime,
				 NULL );

				return( -1 );
			}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
			result = libfdatetime_filetime_copy_to_utf16_string(
			          filetime,
			          (uint16_t *) filetime_string,
			          24,
			          LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
			          LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
			          error );
#else
			result = libfdatetime_filetime_copy_to_utf8_string(
			          filetime,
			          (uint8_t *) filetime_string,
			          24,
			          LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
			          LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
			          error );
#endif
			if( result != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy filetime to string.",
				 function );

				libfdatetime_filetime_free(
				 &filetime,
				 NULL );

				return( -1 );
			}
			if( libfdatetime_filetime_free(
			     &filetime,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free filetime.",
				 function );

				return( -1 );
			}
			fprintf(
			 table_file_stream,
			 "%" PRIs_LIBCSTRING_SYSTEM "",
			 filetime_string );
		}
	}
	else
	{
		if( value_data != NULL )
		{
			while( value_data_size > 0 )
			{
				fprintf(
				 table_file_stream,
				 "%02" PRIx8 "",
				 *value_data );

				value_data      += 1;
				value_data_size -= 1;
			}
		}
	}
	return( 1 );
}

/* Exports a GUID value in a binary data table record value
 * Returns 1 if successful or -1 on error
 */
int exchange_export_record_value_guid(
     libesedb_record_t *record,
     int record_value_entry,
     uint8_t byte_order,
     FILE *table_file_stream,
     liberror_error_t **error )
{
	libcstring_system_character_t guid_string[ LIBFGUID_IDENTIFIER_STRING_SIZE ];

	libfguid_identifier_t *guid = NULL;
	uint8_t *value_data         = NULL;
	static char *function       = "exchange_export_record_value_guid";
	size_t value_data_size      = 0;
	uint32_t column_type        = 0;
	uint8_t value_flags         = 0;
	int result                  = 0;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( table_file_stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table file stream.",
		 function );

		return( -1 );
	}
	if( libesedb_record_get_column_type(
	     record,
	     record_value_entry,
	     &column_type,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve column type of value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( column_type != LIBESEDB_COLUMN_TYPE_BINARY_DATA )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported column type: %" PRIu32 "",
		 function,
		 column_type );

		return( -1 );
	}
	if( libesedb_record_get_value(
	     record,
	     record_value_entry,
	     &value_data,
	     &value_data_size,
	     &value_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( ( value_flags & ~( LIBESEDB_VALUE_FLAG_VARIABLE_SIZE ) ) == 0 )
	{
		if( value_data != NULL )
		{
			if( value_data_size != 16 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
				 "%s: unsupported value data size: %" PRIzd "",
				 function,
				 value_data_size );

				return( -1 );
			}
			if( libfguid_identifier_initialize(
			     &guid,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create GUID.",
				 function );

				return( -1 );
			}
			if( libfguid_identifier_copy_from_byte_stream(
			     guid,
			     value_data,
			     value_data_size,
			     byte_order,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy byte stream to GUID.",
				 function );

				libfguid_identifier_free(
				 &guid,
				 NULL );

				return( -1 );
			}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
			result = libfguid_identifier_copy_to_utf16_string(
			          guid,
			          (uint16_t *) guid_string,
			          LIBFGUID_IDENTIFIER_STRING_SIZE,
			          error );
#else
			result = libfguid_identifier_copy_to_utf8_string(
			          guid,
			          (uint8_t *) guid_string,
			          LIBFGUID_IDENTIFIER_STRING_SIZE,
			          error );
#endif
			if( result != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy GUID to string.",
				 function );

				libfguid_identifier_free(
				 &guid,
				 NULL );

				return( -1 );
			}
			if( libfguid_identifier_free(
			     &guid,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free GUID.",
				 function );

				return( -1 );
			}
			fprintf(
			 table_file_stream,
			 "%" PRIs_LIBCSTRING_SYSTEM "",
			 guid_string );
		}
	}
	else
	{
		if( value_data != NULL )
		{
			while( value_data_size > 0 )
			{
				fprintf(
				 table_file_stream,
				 "%02" PRIx8 "",
				 *value_data );

				value_data      += 1;
				value_data_size -= 1;
			}
		}
	}
	return( 1 );
}

/* Exports a SID value in a binary data table record value
 * Returns 1 if successful or -1 on error
 */
int exchange_export_record_value_sid(
     libesedb_record_t *record,
     int record_value_entry,
     FILE *table_file_stream,
     liberror_error_t **error )
{
	libcstring_system_character_t sid_string[ 128 ];

	libfwnt_security_identifier_t *sid = NULL;
	uint8_t *value_data                = NULL;
	static char *function              = "exchange_export_record_value_sid";
	size_t sid_string_size             = 0;
	size_t value_data_size             = 0;
	uint32_t column_type               = 0;
	uint8_t value_flags                = 0;
	int result                         = 0;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( table_file_stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table file stream.",
		 function );

		return( -1 );
	}
	if( libesedb_record_get_column_type(
	     record,
	     record_value_entry,
	     &column_type,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve column type of value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( column_type != LIBESEDB_COLUMN_TYPE_BINARY_DATA )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported column type: %" PRIu32 "",
		 function,
		 column_type );

		return( -1 );
	}
	if( libesedb_record_get_value(
	     record,
	     record_value_entry,
	     &value_data,
	     &value_data_size,
	     &value_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( ( value_flags & ~( LIBESEDB_VALUE_FLAG_VARIABLE_SIZE ) ) == 0 )
	{
		if( value_data != NULL )
		{
			if( libfwnt_security_identifier_initialize(
			     &sid,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create SID.",
				 function );

				return( -1 );
			}
			if( libfwnt_security_identifier_copy_from_byte_stream(
			     sid,
			     value_data,
			     value_data_size,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy byte stream to SID.",
				 function );

				libfwnt_security_identifier_free(
				 &sid,
				 NULL );

				return( -1 );
			}
			result = libfwnt_security_identifier_get_string_size(
				  sid,
				  &sid_string_size,
				  error );

			if( result != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_GET_FAILED,
				 "%s: unable to retrieve SID string size.",
				 function );

				libfwnt_security_identifier_free(
				 &sid,
				 NULL );

				return( -1 );
			}
			/* It is assumed that the SID string cannot be larger than 127 characters
			 * otherwise using dynamic allocation is more appropriate
			 */
			if( sid_string_size > 128 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
				 "%s: SID string size value exceeds maximum.",
				 function );

				libfwnt_security_identifier_free(
				 &sid,
				 NULL );

				return( -1 );
			}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
			result = libfwnt_security_identifier_copy_to_utf16_string(
			          sid,
			          (uint16_t *) sid_string,
			          128,
			          error );
#else
			result = libfwnt_security_identifier_copy_to_utf8_string(
			          sid,
			          (uint8_t *) sid_string,
			          128,
			          error );
#endif
			if( result != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
				 "%s: unable to copy SID to string.",
				 function );

				libfwnt_security_identifier_free(
				 &sid,
				 NULL );

				return( -1 );
			}
			if( libfwnt_security_identifier_free(
			     &sid,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free SID.",
				 function );

				return( -1 );
			}
			fprintf(
			 table_file_stream,
			 "%" PRIs_LIBCSTRING_SYSTEM "",
			 sid_string );
		}
	}
	else
	{
		if( value_data != NULL )
		{
			while( value_data_size > 0 )
			{
				fprintf(
				 table_file_stream,
				 "%02" PRIx8 "",
				 *value_data );

				value_data      += 1;
				value_data_size -= 1;
			}
		}
	}
	return( 1 );
}

/* Exports a string in a binary data table record value
 * Returns 1 if successful or -1 on error
 */
int exchange_export_record_value_string(
     libesedb_record_t *record,
     int record_value_entry,
     FILE *table_file_stream,
     liberror_error_t **error )
{
	uint8_t *value_data      = NULL;
	uint8_t *value_string    = NULL;
	static char *function    = "exchange_export_record_value_string";
	size_t value_data_size   = 0;
	size_t value_string_size = 0;
	uint32_t column_type     = 0;
	uint8_t value_flags      = 0;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( table_file_stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table file stream.",
		 function );

		return( -1 );
	}
	if( libesedb_record_get_column_type(
	     record,
	     record_value_entry,
	     &column_type,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve column type of value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( ( column_type != LIBESEDB_COLUMN_TYPE_BINARY_DATA )
	 && ( column_type != LIBESEDB_COLUMN_TYPE_LARGE_BINARY_DATA ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported column type: %" PRIu32 "",
		 function,
		 column_type );

		return( -1 );
	}
	if( libesedb_record_get_value(
	     record,
	     record_value_entry,
	     &value_data,
	     &value_data_size,
	     &value_flags,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve value: %d.",
		 function,
		 record_value_entry );

		return( -1 );
	}
	if( value_data != NULL )
	{
		while( value_data_size > 0 )
		{
			fprintf(
			 table_file_stream,
			 "%c",
			 (char) *value_data );

			value_data      += 1;
			value_data_size -= 1;
		}
	}
	return( 1 );
}

/* Exports the values in a Folders table record
 * Returns 1 if successful or -1 on error
 */
int exchange_export_record_folders(
     libesedb_record_t *record,
     FILE *table_file_stream,
     liberror_error_t **error )
{
	libcstring_system_character_t column_name[ 256 ];

	static char *function   = "exchange_export_record_folders";
	size_t column_name_size = 0;
	uint32_t column_type    = 0;
	uint8_t byte_order      = _BYTE_STREAM_ENDIAN_LITTLE;
	int known_column_type   = 0;
	int number_of_values    = 0;
	int result              = 0;
	int value_iterator      = 0;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( table_file_stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table file stream.",
		 function );

		return( -1 );
	}
	if( libesedb_record_get_number_of_values(
	     record,
	     &number_of_values,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of values.",
		 function );

		return( -1 );
	}
	for( value_iterator = 0;
	     value_iterator < number_of_values;
	     value_iterator++ )
	{
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libesedb_record_get_utf16_column_name_size(
		          record,
		          value_iterator,
		          &column_name_size,
		          error );

#else
		result = libesedb_record_get_utf8_column_name_size(
		          record,
		          value_iterator,
		          &column_name_size,
		          error );
#endif
		if( result != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve column name size of value: %d.",
			 function,
			 value_iterator );

			return( -1 );
		}
		/* It is assumed that the column name cannot be larger than 255 characters
		 * otherwise using dynamic allocation is more appropriate
		 */
		if( column_name_size > 256 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
			 "%s: column name size value exceeds maximum.",
			 function );

			return( -1 );
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libesedb_record_get_utf16_column_name(
		          record,
		          value_iterator,
		          (uint16_t *) column_name,
		          column_name_size,
		          error );
#else
		result = libesedb_record_get_utf8_column_name(
		          record,
		          value_iterator,
		          (uint8_t *) column_name,
		          column_name_size,
		          error );
#endif
		if( result != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve column name of value: %d.",
			 function,
			 value_iterator );

			return( -1 );
		}
		if( libesedb_record_get_column_type(
		     record,
		     value_iterator,
		     &column_type,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve column type of value: %d.",
			 function,
			 value_iterator );

			return( -1 );
		}
		known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_UNDEFINED;

		if( column_type == LIBESEDB_COLUMN_TYPE_CURRENCY )
		{
			if( ( column_name_size > 1 )
			 && ( column_name_size <= 6 ) )
			{
				if( column_name[ 0 ] == (libcstring_character_t) 'T' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_FILETIME;
				}
				else if( column_name[ 0 ] == (libcstring_character_t) 'Q' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_64BIT;
				}
			}
		}
		else if( ( column_type == LIBESEDB_COLUMN_TYPE_BINARY_DATA )
		      || ( column_type == LIBESEDB_COLUMN_TYPE_LARGE_BINARY_DATA ) )
		{
			if( ( column_name_size > 1 )
			 && ( column_name_size <= 6 ) )
			{
				if( column_name[ 0 ] == (libcstring_character_t) 'L' )
				{
/* TODO
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_32BIT;
*/
				}
				else if( column_name[ 0 ] == (libcstring_character_t) 'S' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_STRING;
				}
				else if( column_name[ 0 ] == (libcstring_character_t) 'T' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_FILETIME;
				}
				else if( column_name[ 0 ] == (libcstring_character_t) 'Q' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_64BIT;
				}
				else if( column_name_size == 5 )
				{
					if( libcstring_system_string_compare(
					     column_name,
					     _LIBCSTRING_SYSTEM_STRING( "Ne58" ),
					     4 ) == 0 )
					{
						known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_SID;
					}
					else if( libcstring_system_string_compare(
					          column_name,
					          _LIBCSTRING_SYSTEM_STRING( "Ne59" ),
					          4 ) == 0 )
					{
						known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_SID;
					}
				}
				else if( column_name_size == 6 )
				{
					if( libcstring_system_string_compare(
					     column_name,
					     _LIBCSTRING_SYSTEM_STRING( "N3880" ),
					     5 ) == 0 )
					{
						known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_GUID;
					}
				}
			}
		}
		if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_32BIT )
		{
			result = exchange_export_record_value_32bit(
				  record,
				  value_iterator,
				  byte_order,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_64BIT )
		{
			result = exchange_export_record_value_64bit(
				  record,
				  value_iterator,
				  byte_order,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_FILETIME )
		{
			result = exchange_export_record_value_filetime(
				  record,
				  value_iterator,
				  byte_order,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_GUID )
		{
			result = exchange_export_record_value_guid(
				  record,
				  value_iterator,
				  byte_order,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_SID )
		{
			result = exchange_export_record_value_sid(
				  record,
				  value_iterator,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_STRING )
		{
			result = exchange_export_record_value_string(
				  record,
				  value_iterator,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_UNDEFINED )
		{
			result = export_handle_export_record_value(
				  record,
				  value_iterator,
				  table_file_stream,
				  error );
		}
		if( result != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GENERIC,
			 "%s: unable to export record value: %d.",
			 function,
			 value_iterator );

			return( -1 );
		}
		if( value_iterator == ( number_of_values - 1 ) )
		{
			fprintf(
			 table_file_stream,
			 "\n" );
		}
		else
		{
			fprintf(
			 table_file_stream,
			 "\t" );
		}
	}
	return( 1 );
}

/* Exports the values in a Mailbox table record
 * Returns 1 if successful or -1 on error
 */
int exchange_export_record_mailbox(
     libesedb_record_t *record,
     FILE *table_file_stream,
     liberror_error_t **error )
{
	libcstring_system_character_t column_name[ 256 ];

	static char *function   = "exchange_export_record_mailbox";
	size_t column_name_size = 0;
	uint32_t column_type    = 0;
	uint8_t byte_order      = _BYTE_STREAM_ENDIAN_LITTLE;
	int known_column_type   = 0;
	int number_of_values    = 0;
	int result              = 0;
	int value_iterator      = 0;

	if( record == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid record.",
		 function );

		return( -1 );
	}
	if( table_file_stream == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid table file stream.",
		 function );

		return( -1 );
	}
	if( libesedb_record_get_number_of_values(
	     record,
	     &number_of_values,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of values.",
		 function );

		return( -1 );
	}
	for( value_iterator = 0;
	     value_iterator < number_of_values;
	     value_iterator++ )
	{
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libesedb_record_get_utf16_column_name_size(
		          record,
		          value_iterator,
		          &column_name_size,
		          error );

#else
		result = libesedb_record_get_utf8_column_name_size(
		          record,
		          value_iterator,
		          &column_name_size,
		          error );
#endif
		if( result != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve column name size of value: %d.",
			 function,
			 value_iterator );

			return( -1 );
		}
		/* It is assumed that the column name cannot be larger than 255 characters
		 * otherwise using dynamic allocation is more appropriate
		 */
		if( column_name_size > 256 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
			 "%s: column name size value exceeds maximum.",
			 function );

			return( -1 );
		}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
		result = libesedb_record_get_utf16_column_name(
		          record,
		          value_iterator,
		          (uint16_t *) column_name,
		          column_name_size,
		          error );
#else
		result = libesedb_record_get_utf8_column_name(
		          record,
		          value_iterator,
		          (uint8_t *) column_name,
		          column_name_size,
		          error );
#endif
		if( result != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve column name of value: %d.",
			 function,
			 value_iterator );

			return( -1 );
		}
		if( libesedb_record_get_column_type(
		     record,
		     value_iterator,
		     &column_type,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve column type of value: %d.",
			 function,
			 value_iterator );

			return( -1 );
		}
		known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_UNDEFINED;

		if( column_type == LIBESEDB_COLUMN_TYPE_CURRENCY )
		{
			if( ( column_name_size > 1 )
			 && ( column_name_size <= 6 ) )
			{
				if( column_name[ 0 ] == (libcstring_character_t) 'T' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_FILETIME;
				}
				else if( column_name[ 0 ] == (libcstring_character_t) 'Q' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_64BIT;
				}
			}
		}
		else if( ( column_type == LIBESEDB_COLUMN_TYPE_BINARY_DATA )
		      || ( column_type == LIBESEDB_COLUMN_TYPE_LARGE_BINARY_DATA ) )
		{
			if( ( column_name_size > 1 )
			 && ( column_name_size <= 6 ) )
			{
				if( column_name[ 0 ] == (libcstring_character_t) 'L' )
				{
/* TODO
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_32BIT;
*/
				}
				else if( column_name[ 0 ] == (libcstring_character_t) 'S' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_STRING;
				}
				else if( column_name[ 0 ] == (libcstring_character_t) 'T' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_FILETIME;
				}
				else if( column_name[ 0 ] == (libcstring_character_t) 'Q' )
				{
					known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_64BIT;
				}
				else if( column_name_size == 6 )
				{
					if( libcstring_system_string_compare(
					     column_name,
					     _LIBCSTRING_SYSTEM_STRING( "N66a0" ),
					     5 ) == 0 )
					{
						known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_SID;
					}
					else if( libcstring_system_string_compare(
					          column_name,
					          _LIBCSTRING_SYSTEM_STRING( "N676a" ),
					          5 ) == 0 )
					{
						known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_GUID;
					}
					else if( libcstring_system_string_compare(
					          column_name,
					          _LIBCSTRING_SYSTEM_STRING( "N676c" ),
					          5 ) == 0 )
					{
						known_column_type = EXCHANGE_KNOWN_COLUMN_TYPE_GUID;
					}
				}
			}
		}
		if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_32BIT )
		{
			result = exchange_export_record_value_32bit(
				  record,
				  value_iterator,
				  byte_order,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_INTEGER_64BIT )
		{
			result = exchange_export_record_value_64bit(
				  record,
				  value_iterator,
				  byte_order,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_FILETIME )
		{
			result = exchange_export_record_value_filetime(
				  record,
				  value_iterator,
				  byte_order,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_GUID )
		{
			result = exchange_export_record_value_guid(
				  record,
				  value_iterator,
				  byte_order,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_SID )
		{
			result = exchange_export_record_value_sid(
				  record,
				  value_iterator,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_STRING )
		{
			result = exchange_export_record_value_string(
				  record,
				  value_iterator,
				  table_file_stream,
				  error );
		}
		else if( known_column_type == EXCHANGE_KNOWN_COLUMN_TYPE_UNDEFINED )
		{
			result = export_handle_export_record_value(
				  record,
				  value_iterator,
				  table_file_stream,
				  error );
		}
		if( result != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GENERIC,
			 "%s: unable to export record value: %d.",
			 function,
			 value_iterator );

			return( -1 );
		}
		if( value_iterator == ( number_of_values - 1 ) )
		{
			fprintf(
			 table_file_stream,
			 "\n" );
		}
		else
		{
			fprintf(
			 table_file_stream,
			 "\t" );
		}
	}
	return( 1 );
}
