#ifndef GREYOUTOVERLAY_H
#define GREYOUTOVERLAY_H

#include <QPropertyAnimation>
#include <QWidget>

/*!
 * \copyright
 * Copyright (C) 2015 - 2020.
 * Released under the GNU General Public License.
 */

/*!
 * \brief The GreyOutOverlay class is a QWidget that can be applied over any UI and it provides a
 * black layer with transparency that gives the effect of greying out everything below it. It is
 * useful for making a user to focus on a popup window that shows up over the rest fo the UI.
 */
class GreyOutOverlay : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit GreyOutOverlay(bool startAsGreyedOut, QWidget* parent);

    /// fades in and out the greyout
    void greyOut(bool shouldGrey);

    /// set the grey out to a value between 0 and 100, without transition. 100 is fully greyed out,
    /// 0 is no greyout.
    void partialGreyOut(std::uint32_t value);

    /*!
     * \brief resize resize the grey out overlay. should be called on the resizeEvent of whatever
     * the parent of this overlay is.
     */
    void resize();

signals:
    /// emitted when clicked
    void clicked();

private slots:

    /// slot called when greyout fade in is complete.
    void greyOutFadeInComplete();

    /// slot called when greyout fade in is complete.
    void greyOutFadeOutComplete();

    /// slot called when partial grey out completes
    void partialGreyComplete();

protected:
    /// paints the greyout overlay
    void paintEvent(QPaintEvent*);

    /*!
     * \brief mouseReleaseEvent picks up when a click (or a tap on mobile) is released.
     */
    virtual void mouseReleaseEvent(QMouseEvent*);

private:
    /// creates a QPropertyAnimation where it fades from start to end in transTime number of
    /// milliseconds
    QPropertyAnimation* fadeTransition(float start, float end, int transTime);

    /// true if currently greying out, false otherwise
    bool mInTransition;

    /// grey level, where 0 is no grey out and 100 is maximum grey out.
    std::uint32_t mGreyLevel;
};

#endif // GREYOUTOVERLAY_H
