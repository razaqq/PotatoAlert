// Copyright 2020 <github.com/razaqq>

#include "Client/Screenshot.hpp"

#include "Core/Directory.hpp"
#include "Core/Format.hpp"
#include "Core/Log.hpp"
#include "Core/Time.hpp"

#include <QDir>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>

#include <filesystem>
#include <string>


namespace fs = std::filesystem;

static constexpr std::string_view g_timeFormat = "%Y-%m-%d_%H-%M-%S";

namespace {

static std::string GetFileName()
{
	return fmt::format("capture_{}.png", PotatoAlert::Core::Time::GetTimeStamp(g_timeFormat));
}

}

bool PotatoAlert::Client::CaptureScreenshot(QWidget* window, const fs::path& dir, const QList<QRect>& blurRects)
{
	if (!window)
		return false;

	QPixmap pix = window->grab();
	const QRect rect = pix.rect();
	const QSize size = pix.size();

	if (!blurRects.empty())
	{
		QGraphicsScene* scene = new QGraphicsScene(rect);
		scene->addPixmap(pix);

		QGraphicsBlurEffect* effect = new QGraphicsBlurEffect();
		effect->setBlurHints(QGraphicsBlurEffect::BlurHint::QualityHint);
		effect->setBlurRadius(15);

		QGraphicsScene* blurScene = new QGraphicsScene(rect);
		QGraphicsPixmapItem* pixItem = blurScene->addPixmap(pix);
		pixItem->setGraphicsEffect(effect);

		QGraphicsView* blurView = new QGraphicsView(blurScene);
		blurView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		blurView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		QPixmap blurPix(size);
		QPainter blurPainter(&blurPix);
		blurView->render(&blurPainter, blurPix.rect(), rect);

		for (const QRect& r : blurRects)
		{
			QGraphicsPixmapItem* item = scene->addPixmap(blurPix.copy(r));
			item->setOffset(r.topLeft());
		}

		QGraphicsView* view = new QGraphicsView(scene);
		view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		QPainter pixPainter(&pix);
		view->render(&pixPainter, pix.rect(), rect);
	}

	const std::string fileName = GetFileName();
	if (pix.save(QDir(dir).absoluteFilePath(fileName.c_str()), "PNG", 100))
	{
		LOG_TRACE("Saved screenshot {}", fileName);
		return true;
	}
	LOG_ERROR("Failed to save screenshot.");
	return false;
}
