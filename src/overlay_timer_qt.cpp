#include <QApplication>
#include <QScreen>
#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QFont>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainterPath>
#include <QFontDatabase>
#include <cmath>

class OverlayTimer : public QWidget
{
	Q_OBJECT
public:
	OverlayTimer(int seconds, const QString& prompt, QWidget* parent = 0)
		: QWidget(parent), totalSeconds(seconds), remaining(seconds), askConfirm(false), fadeOut(false), promptMsg(prompt)
	{
		setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
		setAttribute(Qt::WA_ShowWithoutActivating, false);
		setFocusPolicy(Qt::StrongFocus);
		
		// Animation for pulsing effect
		pulseAnimation = new QPropertyAnimation(this, "pulseValue");
		pulseAnimation->setDuration(2000);
		pulseAnimation->setStartValue(0.8);
		pulseAnimation->setEndValue(1.0);
		pulseAnimation->setEasingCurve(QEasingCurve::InOutSine);
		pulseAnimation->setLoopCount(-1);
		pulseAnimation->start();

		// Timer
		QTimer* timer = new QTimer(this);
		connect(timer, &QTimer::timeout, this, &OverlayTimer::tick);
		timer->start(1000);

		// Fade out animation
		animation = new QPropertyAnimation(this, "windowOpacity");
		animation->setDuration(800);
		animation->setStartValue(1.0);
		animation->setEndValue(0.0);
		connect(animation, &QPropertyAnimation::finished, this, &OverlayTimer::fadeFinished);

		pulseValue = 0.8;
	}

	void startFadeOut()
	{
		fadeOut = true;
		pulseAnimation->stop();
		animation->start();
	}

	Q_PROPERTY(qreal pulseValue READ getPulseValue WRITE setPulseValue)

	qreal getPulseValue() const { return pulseValue; }
	void setPulseValue(qreal value) { pulseValue = value; update(); }

signals:
	void closeAll();

private slots:
	void tick()
	{
		remaining--;
		if (remaining <= 0) {
			startFadeOut();
		}
		update();
	}

	void fadeFinished()
	{
		emit closeAll();
	}

protected:
	void showEvent(QShowEvent* event)
	{
		QWidget::showEvent(event);
		raise();
		activateWindow();
		setFocus();
		grabKeyboard();
		grabMouse();
	}

	void paintEvent(QPaintEvent*)
	{
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing, true);
		p.setRenderHint(QPainter::SmoothPixmapTransform, true);

		// Enhanced dark background with dynamic particles
		QLinearGradient bgGradient(0, 0, 0, height());
		bgGradient.setColorAt(0, QColor(5, 8, 18, 250));
		bgGradient.setColorAt(0.2, QColor(10, 15, 30, 255));
		bgGradient.setColorAt(0.5, QColor(15, 22, 40, 255));
		bgGradient.setColorAt(0.8, QColor(12, 18, 35, 255));
		bgGradient.setColorAt(1, QColor(8, 12, 25, 250));
		p.fillRect(rect(), bgGradient);

		// Ambient particles effect
		drawAmbientParticles(p);

		// Calculate responsive dimensions
		int centerX = width() / 2;
		int centerY = height() / 2;
		int containerSize = qMin(width(), height()) * 0.7; // Increased to 70%
		int timerRadius = containerSize * 0.32; // Adjusted for better proportions
		
		// Enhanced main container
		drawEnhancedMainContainer(p, centerX, centerY, containerSize);
		
		// Sophisticated progress system
		drawSophisticatedProgressSystem(p, centerX, centerY, timerRadius);
		
		// Premium time display
		drawPremiumTimeDisplay(p, centerX, centerY, containerSize);
		
		// Elegant status and branding
		drawElegantStatusAndBranding(p, centerX, centerY, containerSize);
		
		// Confirmation prompt
		if (askConfirm) {
			drawPremiumConfirmationPrompt(p, centerX, centerY, containerSize);
		}
	}

private:
	void drawAmbientParticles(QPainter& p)
	{
		// Floating particles for ambience
		p.setPen(Qt::NoPen);
		for (int i = 0; i < 30; i++) {
			int x = (i * 127 + (int)(pulseValue * 50)) % width();
			int y = (i * 97 + (int)(pulseValue * 30)) % height();
			int alpha = 10 + (int)(15 * sin(pulseValue * 2 + i * 0.5));
			p.setBrush(QColor(120, 160, 255, alpha));
			p.drawEllipse(x, y, 3, 3);
		}
	}

	void drawEnhancedMainContainer(QPainter& p, int centerX, int centerY, int containerSize)
	{
		// Multiple layer glow system
		for (int layer = 30; layer > 0; layer--) {
			int alpha = qMax(1, 12 - (layer / 3));
			QColor glowColor;
			if (remaining < totalSeconds * 0.3) {
				glowColor = QColor(255, 100, 100, alpha); // Red urgency glow
			} else if (remaining < totalSeconds * 0.6) {
				glowColor = QColor(255, 180, 80, alpha); // Orange warning glow
			} else {
				glowColor = QColor(80, 180, 255, alpha); // Blue calm glow
			}
			
			p.setBrush(glowColor);
			p.setPen(Qt::NoPen);
			int size = containerSize + layer * 6;
			p.drawEllipse(centerX - size/2, centerY - size/2, size, size);
		}
		
		// Main container with enhanced depth
		QRadialGradient containerGradient(centerX, centerY - containerSize/8, containerSize/2.5);
		containerGradient.setColorAt(0, QColor(45, 55, 80, 180));
		containerGradient.setColorAt(0.4, QColor(35, 45, 70, 220));
		containerGradient.setColorAt(0.7, QColor(25, 35, 60, 240));
		containerGradient.setColorAt(0.9, QColor(15, 25, 50, 250));
		containerGradient.setColorAt(1, QColor(10, 20, 45, 255));
		
		p.setBrush(containerGradient);
		p.setPen(QPen(QColor(100, 150, 255, 80), 3));
		p.drawEllipse(centerX - containerSize/2, centerY - containerSize/2, containerSize, containerSize);
		
		// Inner highlight system
		QRadialGradient innerHighlight(centerX, centerY - containerSize/5, containerSize/3);
		innerHighlight.setColorAt(0, QColor(255, 255, 255, 35));
		innerHighlight.setColorAt(0.3, QColor(255, 255, 255, 20));
		innerHighlight.setColorAt(0.7, QColor(255, 255, 255, 8));
		innerHighlight.setColorAt(1, QColor(255, 255, 255, 0));
		
		p.setBrush(innerHighlight);
		p.setPen(Qt::NoPen);
		int highlightSize = containerSize - 40;
		p.drawEllipse(centerX - highlightSize/2, centerY - highlightSize/2, highlightSize, highlightSize);
		
		// Subtle rim details
		p.setPen(QPen(QColor(150, 200, 255, 40), 1));
		p.setBrush(Qt::NoBrush);
		int rimSize = containerSize - 10;
		p.drawEllipse(centerX - rimSize/2, centerY - rimSize/2, rimSize, rimSize);
	}

	void drawSophisticatedProgressSystem(QPainter& p, int centerX, int centerY, int radius)
	{
		// Enhanced background track with gradient
		QConicalGradient trackGradient(centerX, centerY, 0);
		trackGradient.setColorAt(0, QColor(255, 255, 255, 25));
		trackGradient.setColorAt(0.5, QColor(255, 255, 255, 8));
		trackGradient.setColorAt(1, QColor(255, 255, 255, 25));
		
		p.setPen(QPen(trackGradient, 10, Qt::SolidLine, Qt::RoundCap));
		p.setBrush(Qt::NoBrush);
		p.drawEllipse(centerX - radius, centerY - radius, radius * 2, radius * 2);
		
		// Progress calculation
		double progress = (double)remaining / (double)totalSeconds;
		double angle = 360.0 * progress;
		
		// Dynamic progress colors with smooth transitions
		QConicalGradient progressGradient(centerX, centerY, -90);
		
		if (remaining > totalSeconds * 0.7) {
			// Vibrant success gradient
			progressGradient.setColorAt(0, QColor(80, 255, 200, 255));
			progressGradient.setColorAt(0.3, QColor(100, 220, 255, 255));
			progressGradient.setColorAt(0.6, QColor(140, 180, 255, 255));
			progressGradient.setColorAt(1, QColor(160, 140, 255, 255));
		} else if (remaining > totalSeconds * 0.4) {
			// Energetic warning gradient
			progressGradient.setColorAt(0, QColor(255, 240, 100, 255));
			progressGradient.setColorAt(0.3, QColor(255, 200, 120, 255));
			progressGradient.setColorAt(0.7, QColor(255, 160, 140, 255));
			progressGradient.setColorAt(1, QColor(255, 180, 160, 255));
		} else {
			// Intense urgency gradient with pulsing
			int intensity = 220 + (int)(35 * pulseValue);
			progressGradient.setColorAt(0, QColor(255, 100, 100, intensity));
			progressGradient.setColorAt(0.4, QColor(255, 130, 90, intensity));
			progressGradient.setColorAt(0.8, QColor(255, 120, 120, intensity));
			progressGradient.setColorAt(1, QColor(255, 140, 140, intensity));
		}
		
		// Multi-layer progress ring with depth
		for (int layer = 0; layer < 5; layer++) {
			int strokeWidth = 10 - layer;
			int alpha = 255 - (layer * 35);
			
			QPen progressPen(progressGradient, strokeWidth, Qt::SolidLine, Qt::RoundCap);
			QColor penColor = progressPen.color();
			penColor.setAlpha(alpha);
			progressPen.setColor(penColor);
			
			p.setPen(progressPen);
			int layerRadius = radius + layer;
			p.drawArc(centerX - layerRadius, centerY - layerRadius, layerRadius * 2, layerRadius * 2, 
					 90 * 16, -angle * 16);
		}
		
		// Enhanced progress indicator with trail
		if (angle > 0) {
			double dotAngle = (angle - 90) * M_PI / 180.0;
			int dotX = centerX + (radius + 8) * cos(dotAngle);
			int dotY = centerY + (radius + 8) * sin(dotAngle);
			
			// Trailing effect
			for (int i = 1; i <= 5; i++) {
				double trailAngle = (angle - 90 - i * 3) * M_PI / 180.0;
				int trailX = centerX + (radius + 8) * cos(trailAngle);
				int trailY = centerY + (radius + 8) * sin(trailAngle);
				int trailAlpha = 150 - i * 25;
				
				QRadialGradient trailGradient(trailX, trailY, 6);
				trailGradient.setColorAt(0, QColor(255, 255, 255, trailAlpha));
				trailGradient.setColorAt(1, QColor(255, 255, 255, 0));
				
				p.setBrush(trailGradient);
				p.setPen(Qt::NoPen);
				p.drawEllipse(trailX - 6, trailY - 6, 12, 12);
			}
			
			// Main indicator dot
			QRadialGradient dotGradient(dotX, dotY, 12);
			dotGradient.setColorAt(0, QColor(255, 255, 255, 255));
			dotGradient.setColorAt(0.4, QColor(240, 250, 255, 220));
			dotGradient.setColorAt(0.8, QColor(200, 230, 255, 150));
			dotGradient.setColorAt(1, QColor(150, 200, 255, 0));
			
			p.setBrush(dotGradient);
			p.setPen(QPen(QColor(120, 180, 255, 200), 2));
			p.drawEllipse(dotX - 12, dotY - 12, 24, 24);
		}
	}

	void drawPremiumTimeDisplay(QPainter& p, int centerX, int centerY, int containerSize)
	{
		// Calculate responsive font size
		int baseFontSize = containerSize / 7;
		
		// Premium font selection
		QFont timeFont("SF Pro Display", baseFontSize, QFont::Thin);
		QFontDatabase fontDb;
		if (!fontDb.families().contains("SF Pro Display")) {
			timeFont = QFont("Segoe UI", baseFontSize, QFont::Light);
			if (!fontDb.families().contains("Segoe UI")) {
				timeFont = QFont("Arial", baseFontSize, QFont::Normal);
			}
		}
		
		p.setFont(timeFont);
		
		// Enhanced time formatting with better spacing
		QString timeStr = QString("%1:%2")
			.arg(remaining / 60, 2, 10, QChar('0'))
			.arg(remaining % 60, 2, 10, QChar('0'));
		
		// Advanced multi-layer shadow system
		for (int i = 5; i > 0; i--) {
			p.setPen(QColor(0, 0, 0, 80 - i * 12));
			QRect shadowRect(centerX - containerSize/2.5, centerY - baseFontSize/2.5 + i * 2, 
							containerSize * 2/2.5, baseFontSize);
			p.drawText(shadowRect, Qt::AlignCenter, timeStr);
		}
		
		// Premium time text with enhanced gradient
		QLinearGradient timeGradient(0, centerY - baseFontSize/2, 0, centerY + baseFontSize/2);
		timeGradient.setColorAt(0, QColor(255, 255, 255, 255));
		timeGradient.setColorAt(0.2, QColor(250, 252, 255, 250));
		timeGradient.setColorAt(0.5, QColor(240, 245, 255, 245));
		timeGradient.setColorAt(0.8, QColor(220, 235, 255, 240));
		timeGradient.setColorAt(1, QColor(200, 225, 255, 220));
		
		QPen timePen;
		timePen.setBrush(timeGradient);
		timePen.setWidth(1);
		p.setPen(timePen);
		
		QRect timeRect(centerX - containerSize/2.5, centerY - baseFontSize/2.5, 
					  containerSize * 2/2.5, baseFontSize);
		p.drawText(timeRect, Qt::AlignCenter, timeStr);
		
		// Refined time unit labels
		QFont unitFont = timeFont;
		unitFont.setPointSize(baseFontSize / 5);
		unitFont.setWeight(QFont::Light);
		p.setFont(unitFont);
		p.setPen(QColor(180, 200, 230, 140));
		
		QRect unitRect(centerX - containerSize/3, centerY + baseFontSize/3, 
					  containerSize * 2/3, baseFontSize/4);
		p.drawText(unitRect, Qt::AlignCenter, "MINUTES : SECONDS");
	}

	void drawElegantStatusAndBranding(QPainter& p, int centerX, int centerY, int containerSize)
	{
		int titleFontSize = containerSize / 18;
		int subtitleFontSize = containerSize / 26;
		int instructionFontSize = containerSize / 32;
		
		// Elegant break title
		QFont titleFont("SF Pro Display", titleFontSize, QFont::Medium);
		QFontDatabase fontDb;
		if (!fontDb.families().contains("SF Pro Display")) {
			titleFont = QFont("Segoe UI", titleFontSize, QFont::DemiBold);
			if (!fontDb.families().contains("Segoe UI")) {
				titleFont = QFont("Arial", titleFontSize, QFont::Bold);
			}
		}
		
		p.setFont(titleFont);
		
		// Enhanced title gradient
		QLinearGradient titleGradient(0, centerY + containerSize/4, 0, centerY + containerSize/3);
		titleGradient.setColorAt(0, QColor(255, 255, 255, 250));
		titleGradient.setColorAt(0.5, QColor(240, 250, 255, 230));
		titleGradient.setColorAt(1, QColor(220, 240, 255, 210));
		
		QPen titlePen;
		titlePen.setBrush(titleGradient);
		p.setPen(titlePen);
		
		QString breakTitle = "🌱  B R E A K   T I M E  🌱";
		QRect titleRect(centerX - containerSize/1.8, centerY + containerSize/6, 
					   containerSize * 2/1.8, titleFontSize + 15);
		p.drawText(titleRect, Qt::AlignCenter, breakTitle);
		
		// Sophisticated subtitle with icons
		QFont subtitleFont = titleFont;
		subtitleFont.setPointSize(subtitleFontSize);
		subtitleFont.setWeight(QFont::Light);
		p.setFont(subtitleFont);
		p.setPen(QColor(200, 220, 250, 190));
		
		QString subtitle = "🚶 Step Away  •  🤸 Stretch  •  🫁 Breathe  •  💧 Hydrate";
		QRect subtitleRect(centerX - containerSize/1.6, centerY + containerSize/4 + titleFontSize + 5, 
						  containerSize * 2/1.6, subtitleFontSize + 10);
		p.drawText(subtitleRect, Qt::AlignCenter, subtitle);
		
		// Enhanced instruction with dynamic pulsing
		if (!askConfirm) {
			QFont instructionFont = subtitleFont;
			instructionFont.setPointSize(instructionFontSize);
			instructionFont.setWeight(QFont::Normal);
			p.setFont(instructionFont);
			
			// Dynamic pulsing effect
			int alpha = 100 + (int)(80 * pulseValue);
			p.setPen(QColor(180, 200, 240, alpha));
			
			QString instruction = "⚡ Press ESC for urgent interruption";
			QRect instructionRect(centerX - containerSize/1.8, 
								 centerY + containerSize/2.5 + subtitleFontSize, 
								 containerSize * 2/1.8, instructionFontSize + 8);
			p.drawText(instructionRect, Qt::AlignCenter, instruction);
		}
		
		// Enhanced progress percentage with ring
		QFont percentFont = subtitleFont;
		percentFont.setPointSize(subtitleFontSize - 2);
		percentFont.setWeight(QFont::Light);
		p.setFont(percentFont);
		p.setPen(QColor(160, 190, 230, 130));
		
		double progress = (double)remaining / (double)totalSeconds;
		int percentage = (int)(progress * 100);
		QString percentText = QString("⏱ %1% time remaining").arg(percentage);
		
		QRect percentRect(centerX - containerSize/2.5, centerY - containerSize/2.8, 
						 containerSize * 2/2.5, subtitleFontSize);
		p.drawText(percentRect, Qt::AlignCenter, percentText);
	}

	void drawPremiumConfirmationPrompt(QPainter& p, int centerX, int centerY, int containerSize)
	{
		// Enhanced confirmation dialog
		int promptWidth = containerSize * 0.85;
		int promptHeight = containerSize * 0.28;
		QRect promptBg(centerX - promptWidth/2, centerY + containerSize/8, 
					  promptWidth, promptHeight);
		
		// Sophisticated warning background with glow
		for (int i = 8; i > 0; i--) {
			QColor glowColor(255, 150, 100, 15 - i);
			p.setBrush(glowColor);
			p.setPen(Qt::NoPen);
			QRect glowRect = promptBg.adjusted(-i*2, -i*2, i*2, i*2);
			p.drawRoundedRect(glowRect, 25 + i, 25 + i);
		}
		
		QLinearGradient warningGradient(promptBg.topLeft(), promptBg.bottomRight());
		warningGradient.setColorAt(0, QColor(255, 130, 110, 240));
		warningGradient.setColorAt(0.2, QColor(255, 150, 130, 250));
		warningGradient.setColorAt(0.5, QColor(255, 170, 150, 250));
		warningGradient.setColorAt(0.8, QColor(255, 150, 130, 250));
		warningGradient.setColorAt(1, QColor(255, 130, 110, 240));
		
		// Enhanced drop shadow
		p.setBrush(QColor(0, 0, 0, 100));
		p.setPen(Qt::NoPen);
		p.drawRoundedRect(promptBg.translated(8, 8), 25, 25);
		
		// Main prompt background
		p.setBrush(warningGradient);
		p.setPen(QPen(QColor(255, 220, 200, 180), 3));
		p.drawRoundedRect(promptBg, 25, 25);
		
		// Inner highlight
		QLinearGradient innerGlow(promptBg.topLeft(), promptBg.bottomLeft());
		innerGlow.setColorAt(0, QColor(255, 255, 255, 40));
		innerGlow.setColorAt(0.3, QColor(255, 255, 255, 20));
		innerGlow.setColorAt(1, QColor(255, 255, 255, 5));
		
		p.setBrush(innerGlow);
		p.setPen(Qt::NoPen);
		QRect innerRect = promptBg.adjusted(8, 8, -8, -8);
		p.drawRoundedRect(innerRect, 20, 20);
		
		// Premium prompt text
		int promptFontSize = containerSize / 20;
		QFont promptFont("SF Pro Display", promptFontSize, QFont::DemiBold);
		QFontDatabase fontDb;
		if (!fontDb.families().contains("SF Pro Display")) {
			promptFont = QFont("Segoe UI", promptFontSize, QFont::DemiBold);
			if (!fontDb.families().contains("Segoe UI")) {
				promptFont = QFont("Arial", promptFontSize, QFont::Bold);
			}
		}
		
		p.setFont(promptFont);
		p.setPen(QColor(255, 255, 255, 255));
		
		QString msg = "⚠️  " + promptMsg;
		QRect msgRect = promptBg.adjusted(25, 15, -25, -promptHeight/2);
		p.drawText(msgRect, Qt::AlignCenter, msg);
		
		// Enhanced instructions with key indicators
		QFont instFont = promptFont;
		instFont.setPointSize(promptFontSize * 0.75);
		instFont.setWeight(QFont::Normal);
		p.setFont(instFont);
		p.setPen(QColor(255, 255, 255, 230));
		
		QRect instRect = promptBg.adjusted(25, promptHeight/2, -25, -15);
		p.drawText(instRect, Qt::AlignCenter, "🔑 Y = Skip Break  •  🔑 N = Continue Break");
	}

	void keyPressEvent(QKeyEvent* e)
	{
		if (!askConfirm && e->key() == Qt::Key_Escape) {
			askConfirm = true;
			update();
		}
		else if (askConfirm && (e->key() == Qt::Key_Y || e->key() == Qt::Key_Return)) {
			startFadeOut();
		}
		else if (askConfirm && (e->key() == Qt::Key_N || e->key() == Qt::Key_Escape)) {
			askConfirm = false;
			update();
		}
		e->accept();
	}

	void mousePressEvent(QMouseEvent* e) { e->accept(); }
	void mouseReleaseEvent(QMouseEvent* e) { e->accept(); }
	void mouseMoveEvent(QMouseEvent* e) { e->accept(); }
	void wheelEvent(QWheelEvent* e) { e->accept(); }

	void closeEvent(QCloseEvent* e)
	{
		releaseKeyboard();
		releaseMouse();
		QWidget::closeEvent(e);
	}

private:
	int totalSeconds;
	int remaining;
	bool askConfirm;
	bool fadeOut;
	QPropertyAnimation* animation;
	QPropertyAnimation* pulseAnimation;
	QString promptMsg;
	qreal pulseValue;
};

class OverlayManager : public QObject
{
	Q_OBJECT
public:
	OverlayManager(int seconds, const QString& prompt)
	{
		QList<QScreen*> screens = QGuiApplication::screens();
		
		// Create one overlay per screen
		for (int i = 0; i < screens.size(); ++i) {
			QScreen* screen = screens[i];
			QRect geometry = screen->geometry();
			
			OverlayTimer* overlay = new OverlayTimer(seconds, prompt);
			overlay->setGeometry(geometry);
			overlay->show();
			overlay->raise();
			overlay->activateWindow();
			
			// Connect to close all when one closes
			connect(overlay, &OverlayTimer::closeAll, this, &OverlayManager::closeAllOverlays);
			
			overlays.append(overlay);
		}
		
		// Focus first overlay for keyboard input
		if (!overlays.isEmpty()) {
			overlays.first()->setFocus();
		}
	}

public slots:
	void closeAllOverlays()
	{
		for (OverlayTimer* overlay : overlays) {
			overlay->close();
		}
		QCoreApplication::quit();
	}

private:
	QList<OverlayTimer*> overlays;
};

int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <seconds> [prompt]\n", argv[0]);
		return 1;
	}
	
	int seconds = atoi(argv[1]);
	QString overlayPrompt = (argc >= 3) ? argv[2] : "Really urgent to skip? (Y/N)";
	
	QApplication app(argc, argv);
	
	OverlayManager manager(seconds, overlayPrompt);
	
	return app.exec();
}

#include "overlay_timer_qt.moc"
