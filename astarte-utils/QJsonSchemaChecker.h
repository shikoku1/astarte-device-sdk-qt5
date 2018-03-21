/**

MIT License

Copyright (c) 2014 hyperion team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef _QJSONSCHEMACHECKER_H
#define _QJSONSCHEMACHECKER_H

// stl includes
#include <string>
#include <list>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QStringList>

/// JsonSchemaChecker is a very basic implementation of json schema.
/// The json schema definition draft can be found at
/// http://tools.ietf.org/html/draft-zyp-json-schema-03
///
/// The following keywords are supported:
/// - type
/// - required
/// - properties
/// - items
/// - enum
/// - minimum
/// - maximum
/// - addtionalProperties
/// - minItems
/// - maxItems

class QJsonSchemaChecker
{
    public:
        QJsonSchemaChecker();
        virtual ~QJsonSchemaChecker();

        ///
        /// @param schema The schema to use
        /// @return true upon succes
        ///
        bool setSchema(const QJsonObject & schema);

        ///
        /// @brief Validate a JSON structure
        /// @param value The JSON value to check
        /// @return true when the arguments is valid according to the schema
        ///
        bool validate(const QJsonObject & value, bool ignoreRequired = false);

        ///
        /// @return A list of error messages
        ///
        QStringList getMessages() const;

    private:
        ///
        /// Validates a json-value against a given schema. Results are stored in the members of this
        /// class (_error & _messages)
        ///
        /// @param[in] value The value to validate
        /// @param[in] schema The schema against which the value is validated
        ///
        void validate(const QJsonValue &value, const QJsonObject & schema);

        ///
        /// Adds the given message to the message-queue (with reference to current line-number)
        ///
        /// @param[in] message The message to add to the queue
        ///
        void setMessage(const QString & message);

    private:
        // attribute check functions
        ///
        /// Checks if the given value is of the specified type. If the type does not match _error is set
        /// to true and an error-message is added to the message-queue
        ///
        /// @param[in] value The given value
        /// @param[in] schema The specified type (as json-value)
        ///
        void checkType(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if all properties are of the correct format.
        /// If this is not the case _error is set to true and an error-message is added
        /// to the message-queue.
        ///
        /// @param[in] value The given json-object
        /// @param[in] schema The schema of the json-object
        ///
        void checkProperties(const QJsonObject & value, const QJsonObject & schema);

        ///
        /// Verifies the additional configured properties of an json-object. If this is not the case
        /// _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param value The given json-object
        /// @param schema The schema for the json-object
        /// @param ignoredProperties The properties that were ignored
        ///
        void checkAdditionalProperties(const QJsonObject & value, const QJsonValue & schema, const QStringList & ignoredProperties);

        ///
        /// Checks if the given value is larger or equal to the specified value. If this is not the case
        /// _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param[in] value The given value
        /// @param[in] schema The minimum value (as json-value)
        ///
        void checkMinimum(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if the given value is smaller or equal to the specified value. If this is not the
        /// case _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param[in] value The given value
        /// @param[in] schema The maximum value (as json-value)
        ///
        void checkMaximum(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if the given string matches the regex pattern given as the schema. If this is not the
        /// case _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param[in] value The given string
        /// @param[in] schema The regex pattern (as json-value)
        ///
        void checkPattern(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if the given string has a length greater than or equal to the limit given as the schema.
        /// If this is not the case _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param[in] value The given string
        /// @param[in] schema The minimum length (as json-value)
        ///
        void checkMinLength(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if the given string has a length less than or equal to the limit given as the schema.
        /// If this is not the case _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param[in] value The given string
        /// @param[in] schema The maximum length (as json-value)
        ///
        void checkMaxLength(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if the required properties of an object are present. If this is not the
        /// case _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param[in] value The given json-object
        /// @param[in] schema The required array (as json-value)
        ///
        void checkRequired(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Validates all the items of an array.
        ///
        /// @param value The json-array
        /// @param schema The schema for the items in the array
        ///
        void checkItems(const QJsonValue & value, const QJsonObject & schema);

        ///
        /// Checks if a given array has at least a minimum number of items. If this is not the case
        /// _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param value The json-array
        /// @param schema The minimum size specification (as json-value)
        ///
        void checkMinItems(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if a given array has at most a maximum number of items. If this is not the case
        /// _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param value The json-array
        /// @param schema The maximum size specification (as json-value)
        ///
        void checkMaxItems(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if a given array contains only unique items. If this is not the case
        /// _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param value The json-array
        /// @param schema Bool to enable the check (as json-value)
        ///
        void checkUniqueItems(const QJsonValue & value, const QJsonValue & schema);

        ///
        /// Checks if an enum value is actually a valid value for that enum. If this is not the case
        /// _error is set to true and an error-message is added to the message-queue.
        ///
        /// @param value The enum value
        /// @param schema The enum schema definition
        ///
        void checkEnum(const QJsonValue & value, const QJsonValue & schema);

    private:
        /// The schema of the entire json-configuration
        QJsonObject _qSchema;
        /// ignore the required value in json schema
        bool _ignoreRequired;
        /// The current location into a json-configuration structure being checked
        QStringList _currentPath;
        /// The result messages collected during the schema verification
        QStringList _messages;
        /// Flag indicating an error occured during validation
        bool _error;
};

#endif // _QJSONSCHEMACHECKER_H
