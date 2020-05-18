#ifndef UTILS_H
#define UTILS_H

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QTime>
#include <QVector>

QString getTime() { return QTime::currentTime().toString("HH:mm:ss:zzz"); }

QVector<QString> readLines(QFile file)
{
	if (!file.open(QIODevice::ReadOnly))
		return {};

	QVector<QString> tokens;

	QTextStream inputStream(&file);
	while (!inputStream.atEnd())
		tokens.push_back(inputStream.readLine());
	file.close();

	return tokens;
}

#endif // UTILS_H
