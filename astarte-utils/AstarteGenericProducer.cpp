/*
 * This file is part of Astarte.
 *
 * Copyright 2017 Ispirata Srl
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "AstarteGenericProducer.h"

#include <QtCore/QDebug>

AstarteGenericProducer::AstarteGenericProducer(const QByteArray &interface, Hyperdrive::Interface::Type interfaceType,
                                               Hyperdrive::AstarteTransport *astarteTransport, QObject *parent)
    : Hyperspace::ProducerConsumer::ProducerAbstractInterface(interface, astarteTransport, parent)
    , m_interfaceType(interfaceType)
{
}

AstarteGenericProducer::~AstarteGenericProducer()
{
}

QHash<QByteArray, QByteArrayList> AstarteGenericProducer::mappingToTokens() const
{
    return m_mappingToTokens;
}

QHash<QByteArray, QVariant> AstarteGenericProducer::mappingToType() const
{
    return m_mappingToType;
}

bool validateTarget(const QByteArray &target){
    if (!target.startsWith('/') || target.endsWith('/') || target.contains("//")
            || target.contains(";") || target.contains('\n') || target.contains('\r')
            || target.contains('+') || target.contains('#')) {
        return false;
    }
    return true;
}

bool AstarteGenericProducer::sendData(const QVariant &value, const QByteArray &target,
        const QDateTime &timestamp, const QVariantHash &metadata)
{
    if (!validateTarget(target)) {
        qWarning() << "Invalid target: " << target << ". Discarding value: " << value;
        return false;
    }

    QByteArrayList targetTokens = target.mid(1).split('/');
    QHash<QByteArray, QByteArrayList>::const_iterator it;

    for (it = m_mappingToTokens.constBegin(); it != m_mappingToTokens.constEnd(); it++) {
        if (targetTokens.size() != it.value().size()) {
            continue;
        }

        bool mappingMatch = true;
        for (int i = 0; i < targetTokens.size(); ++i) {
            if (it.value().at(i).startsWith("%{")) {
                continue;
            }

            if (targetTokens.at(i) != it.value().at(i)) {
                mappingMatch = false;
            }
        }

        if (!mappingMatch) {
            continue;
        }

        QByteArray matchedMapping = it.key();


        QHash<QByteArray, QByteArray> attributes;
        attributes.insert("interfaceType", QByteArray::number(static_cast<int>(m_interfaceType)));
        if (m_mappingToRetention.contains(matchedMapping)) {
            attributes.insert("retention", QByteArray::number(static_cast<int>(m_mappingToRetention.value(matchedMapping))));
            if (m_mappingToExpiry.contains(matchedMapping)) {
                attributes.insert("expiry", QByteArray::number(m_mappingToExpiry.value(matchedMapping)));
            }
        }

        if (m_mappingToReliability.contains(matchedMapping)) {
            attributes.insert("reliability", QByteArray::number(static_cast<int>(m_mappingToReliability.value(matchedMapping))));
        }


        // array type
        if (value.canConvert<QList<QVariant>>()) {
            QVariant variant = value.value<QList<QVariant>>();
            QList<QVariant> list = variant.toList();

            if (list.length() == 0){
                // if empty, we don't care but we have to send the array
                sendDataOnEndpoint(list, target, attributes, timestamp, metadata);
                return true;
            } else {
                // if there's something in the array

                //this is only a demonstration
                if (std::string(m_mappingToType.value(matchedMapping).typeName()).find(list[0].typeName()) != std::string::npos) {
                    sendDataOnEndpoint(list, target, attributes, timestamp, metadata);
                    return true;
                }
            }
        } else { // scalar type

            if (!value.canConvert(m_mappingToType.value(matchedMapping).type())) {
                qWarning() << "Invalid type for scalar value in sendDataOnEndpoint, expected " << m_mappingToType.value(matchedMapping) << "got" << value.type() << "for " << matchedMapping;
                return false;
            }


            QVariant converted = value;
            converted.convert(m_mappingToType.value(matchedMapping).type());
            switch (converted.type()) {
                case QVariant::Bool:
                    sendDataOnEndpoint(value.toBool(), target, attributes, timestamp, metadata);
                    return true;
                case QVariant::ByteArray:
                    sendDataOnEndpoint(value.toByteArray(), target, attributes, timestamp, metadata);
                    return true;
                case QVariant::DateTime:
                    sendDataOnEndpoint(value.toDateTime(), target, attributes, timestamp, metadata);
                    return true;
                case QVariant::Double:
                    sendDataOnEndpoint(value.toDouble(), target, attributes, timestamp, metadata);
                    return true;
                case QVariant::Int:
                    sendDataOnEndpoint(value.toInt(), target, attributes, timestamp, metadata);
                    return true;
                case QVariant::LongLong:
                    sendDataOnEndpoint(value.toLongLong(), target, attributes, timestamp, metadata);
                    return true;
                case QVariant::String:
                    sendDataOnEndpoint(value.toString(), target, attributes, timestamp, metadata);
                    return true;
                default:
                    qWarning() << "Can't find valid scalar type for " << target;
            }

        }
    }


    qWarning() << "Can't find valid mapping for " << target;
    return false;
}

bool AstarteGenericProducer::sendData(const QVariantHash &value, const QByteArray &target, const QDateTime &timestamp, const QVariantHash &metadata)
{
    // TODO: verify path match
    QHash<QByteArray, QByteArray> attributes;
    attributes.insert("interfaceType", QByteArray::number(static_cast<int>(m_interfaceType)));
    // TODO: handle reliability, retention and expiry

    sendDataOnEndpoint(value, target, attributes, timestamp, metadata);

    return true;
}

bool AstarteGenericProducer::unsetPath(const QByteArray &target)
{
    if (!validateTarget(target)) {
        qWarning() << "Invalid target: " << target << ". Discarding unset";
        return false;
    }

    QByteArrayList targetTokens = target.split('/');
    QHash<QByteArray, bool>::const_iterator it;

    for (it = m_mappingToAllowUnset.constBegin(); it != m_mappingToAllowUnset.constEnd(); it++) {
        if (!it.value()) {
            continue;
        }

        // Split and validate
        QByteArrayList mappingTokens = it.key().split('/');
        if (mappingTokens.size() != targetTokens.size()) {
            continue;
        }
        bool mappingMatches = true;
        for (int i = 0; i < mappingTokens.size(); ++i) {
            if (mappingTokens.at(i).startsWith("%{")) {
                // Assume it is valid
                continue;
            }
            if (mappingTokens.at(i) != targetTokens.at(i)) {
                mappingMatches = false;
                break;
            }
        }

        if (mappingMatches) {
            QHash<QByteArray, QByteArray> attributes;
            attributes.insert("interfaceType", QByteArray::number(static_cast<int>(m_interfaceType)));

            sendRawDataOnEndpoint(QByteArray(), target, attributes);
            return true;
        }
    }

    qWarning() << "Trying to unset " << target << "without allow_unset";
    return false;
}

void AstarteGenericProducer::setMappingToTokens(const QHash<QByteArray, QByteArrayList> &mappingToTokens)
{
    m_mappingToTokens = mappingToTokens;
}

void AstarteGenericProducer::setMappingToType(const QHash<QByteArray, QVariant> &mappingToType)
{
    m_mappingToType = mappingToType;
}

void AstarteGenericProducer::setMappingToRetention(const QHash<QByteArray, Hyperspace::Retention> &mappingToRetention)
{
    m_mappingToRetention = mappingToRetention;
}

void AstarteGenericProducer::setMappingToReliability(const QHash<QByteArray, Hyperspace::Reliability> &mappingToReliability)
{
    m_mappingToReliability = mappingToReliability;
}

void AstarteGenericProducer::setMappingToExpiry(const QHash<QByteArray, int> &mappingToExpiry)
{
    m_mappingToExpiry = mappingToExpiry;
}

void AstarteGenericProducer::setMappingToAllowUnset(const QHash<QByteArray, bool> &mappingToAllowUnset)
{
    m_mappingToAllowUnset = mappingToAllowUnset;
}

void AstarteGenericProducer::populateTokensAndStates()
{
}

Hyperspace::ProducerConsumer::ProducerAbstractInterface::DispatchResult AstarteGenericProducer::dispatch(int i, const QByteArray &payload,
                                                                                                         const QList<QByteArray> &inputTokens)
{
    return IndexNotFound;
}
