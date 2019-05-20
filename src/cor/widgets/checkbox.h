#ifndef COR_CHECK_BOX_H
#define COR_CHECK_BOX_H

#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QWidget>

namespace cor {

/*!
 * \copyright
 * Copyright (C) 2015 - 2019.
 * Released under the GNU General Public License.
 *
 *
 * \brief The CheckBox class is a simple widget designed to give a checkbox
 *        with a label.
 */
class CheckBox : public QWidget {
    Q_OBJECT
public:
    /// constructor
    explicit CheckBox(QWidget* parent, const QString& title) : QWidget(parent) {
        mIsChecked = false;
        mSpacer = 5;

        const QString transparentStyleSheet = "background-color: rgba(0,0,0,0);";

        mTitle = new QLabel(title, this);
        mTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mTitle->setStyleSheet(transparentStyleSheet);
        mTitle->setAlignment(Qt::AlignBottom);
        QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
        mTitle->setFixedWidth(r.width());
        mTitle->setMinimumHeight(int(r.height() * 1.75f));

        mCheckBox = new QPushButton(this);
        mCheckBox->setCheckable(true);
        connect(mCheckBox, SIGNAL(clicked(bool)), this, SLOT(buttonPressed(bool)));
        mCheckBox->setStyleSheet("QPushButton:checked{ background-color:rgb(61, 142, 201); } "
                                 "QPushButton{ border:5px solid #AAAAAA; }");
        mCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        this->setMinimumHeight(mTitle->height());
        mCheckBox->setMinimumHeight(mTitle->height());
        mCheckBox->setMinimumWidth(mTitle->height());
    }


    /// checks and unchecks the checkbox
    void setChecked(bool shouldCheck) {
        mCheckBox->setChecked(shouldCheck);
        mIsChecked = shouldCheck;
    }

    /// set the title for the corluma checkbox
    void setTitle(const QString& title) {
        mTitle->setText(title);
        QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
        mTitle->setFixedWidth(r.width());
        mTitle->setMinimumHeight(int(r.height() * 1.75f));

        this->setMinimumHeight(mTitle->height());
        mCheckBox->setMinimumHeight(mTitle->height());
        mCheckBox->setMinimumWidth(mTitle->height());
    }

    /*!
     * \brief downsizeTextWidthToFit downsize the font's point size until this entire widget
     *        fits into the width provided. If downsizing is not needed, the system's font
     *        size is used instead.
     *
     * NOTE: this is hacky and inefficient!
     * \param maxWidth max width for the entire widget.
     */
    void downsizeTextWidthToFit(int maxWidth) {
        QLabel label(mTitle->text());
        int systemFontWidth = label.fontMetrics().boundingRect(label.text()).width();
        int fontPtSize = label.font().pointSize();
        int nonTitleSize = mCheckBox->width() + mSpacer * 3;
        int computedSize = systemFontWidth;
        maxWidth = maxWidth - nonTitleSize;
        if (maxWidth > computedSize) {
            // just use the systems font instead of scaling up
            QFont font = mTitle->font();
            font.setPointSize(fontPtSize);
            mTitle->setFont(font);
            QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
            mTitle->setFixedWidth(r.width());
        } else {
            while ((maxWidth < computedSize) && (fontPtSize > 2)) {
                fontPtSize--;
                QFont font = mTitle->font();
                font.setPointSize(fontPtSize);
                mTitle->setFont(font);
                QRect r = mTitle->fontMetrics().boundingRect(mTitle->text());
                mTitle->setFixedWidth(r.width());
                computedSize = mTitle->width();
            }
        }
        adjustSize();
    }

    /// getter for whether a box is checked or not.
    bool checked() { return mIsChecked; }

signals:

    /// sent out whenever the checkbox is checked or unchecked
    void boxChecked(bool);

private slots:

    /// handles when the checkbox button is clicked
    void buttonPressed(bool) {
        mIsChecked = !mIsChecked;
        emit boxChecked(mIsChecked);
    }

protected:
    /// resize the widget
    virtual void resizeEvent(QResizeEvent*) {
        mTitle->setGeometry(mSpacer, mSpacer, mTitle->width(), mTitle->height());

        auto height = int(mTitle->height() * 0.8);
        mCheckBox->setGeometry(mTitle->width() + 2 * mSpacer,
                               mTitle->geometry().y(),
                               height + mSpacer,
                               height + mSpacer);

        adjustSize();
    }

private:
    /// true if checked, false if not checked
    bool mIsChecked;

    /// spacer between checkbox and title
    int mSpacer;

    /// label for checkbox
    QLabel* mTitle;

    /// button that displays checked and unchecked states
    QPushButton* mCheckBox;
};

} // namespace cor
#endif // COR_CHECK_BOX_H
