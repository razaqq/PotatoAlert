// Copyright 2022 <github.com/razaqq>

#include "Gui/NativeWindow.hpp"

#include <QSharedMemory>
#include <QLocalSocket>
#include <QLocalServer>


static std::string_view g_sockPath = "/tmp/PotatoAlert-f0a3b58c-b62e-4b3e-a10a-8e2f7a164b7a";
static QLocalServer g_server;

using PotatoAlert::Gui::NativeWindow;

void NativeWindow::RequestFocus()
{
	QLocalSocket socket;
	socket.connectToServer(QString(g_sockPath.data()), QLocalSocket::WriteOnly);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool NativeWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
#else
bool NativeWindow::nativeEvent(const QByteArray& eventType, void* message, long* result)
#endif
{
	if (!g_server.isListening())
	{
		g_server.listen(QString(g_sockPath.data()));
	}

	connect(&g_server, &QLocalServer::newConnection, [this]()
	{
		show();
		setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
		raise();
		activateWindow();
	});

	return QWidget::nativeEvent(eventType, message, result);
}
