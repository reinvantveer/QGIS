/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsserverexception.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsServerException
{
%Docstring
 Exception base class for server exceptions.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsserverexception.h"
%End
  public:
    QgsServerException( const QString &message, int responseCode = 500 );
%Docstring
Constructor
%End

    int responseCode() const;
%Docstring
 :return: the return HTTP response code associated with this exception
 :rtype: int
%End

    virtual QByteArray formatResponse( QString &responseFormat /Out/ ) const;
%Docstring
 Format the exception for sending to client

 \param responseFormat QString to store the content type of the response format.
 :return: QByteArray the fermatted response.

 The defaolt implementation return text/xml format.
 :rtype: QByteArray
%End

};

class QgsOgcServiceException
{
%Docstring
 Exception base class for service exceptions.

 Note that this exception is associated with a default return code 200 which may be
 not appropriate in some situations.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsserverexception.h"
%End
  public:
    QgsOgcServiceException( const QString &code, const QString &message, const QString &locator = QString(),
                            int responseCode = 200, const QString &version = QStringLiteral( "1.3.0" ) );
%Docstring
Construction
%End

    QString message() const;
%Docstring
:return: message
 :rtype: str
%End

    QString code()    const;
%Docstring
:return: code
 :rtype: str
%End

    QString locator() const;
%Docstring
:return: locator
 :rtype: str
%End

    QString version() const;
%Docstring
return exception version
 :rtype: str
%End

    virtual QByteArray formatResponse( QString &responseFormat /Out/ ) const;
%Docstring
Overridden from QgsServerException
 :rtype: QByteArray
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/server/qgsserverexception.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
