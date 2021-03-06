#ifndef BDN_IOS_ViewCore_HH_
#define BDN_IOS_ViewCore_HH_

#import <UIKit/UIKit.h>

#include <bdn/IViewCore.h>
#include <bdn/View.h>
#include <bdn/LayoutCoordinator.h>

#import <bdn/ios/UiProvider.hh>
#import <bdn/ios/util.hh>

#include <bdn/Dip.h>

namespace bdn
{
    namespace ios
    {

        class ViewCore : public Base, BDN_IMPLEMENTS IViewCore, BDN_IMPLEMENTS LayoutCoordinator::IViewCoreExtension
        {
          public:
            ViewCore(View *outerView, UIView *view)
            {
                _outerViewWeak = outerView;
                _view = view;

                _addToParent(outerView->getParentView());

                setVisible(outerView->visible());
                setPadding(outerView->padding());
            }

            ~ViewCore() { _view = nil; }

            P<View> getOuterViewIfStillAttached() const { return _outerViewWeak.toStrong(); }

            UIView *getUIView() const { return _view; }

            void setVisible(const bool &visible) override { _view.hidden = !visible; }

            void setPadding(const Nullable<UiMargin> &padding) override {}

            void setMargin(const UiMargin &margin) override {}

            void invalidateSizingInfo(View::InvalidateReason reason) override
            {
                // nothing to do since we do not cache sizing info in the core.
            }

            void needLayout(View::InvalidateReason reason) override
            {
                P<View> outerView = getOuterViewIfStillAttached();
                if (outerView != nullptr) {
                    P<UiProvider> provider = tryCast<UiProvider>(outerView->getUiProvider());
                    if (provider != nullptr)
                        provider->getLayoutCoordinator()->viewNeedsLayout(outerView);
                }
            }

            void childSizingInfoInvalidated(View *child) override
            {
                P<View> outerView = getOuterViewIfStillAttached();
                if (outerView != nullptr) {
                    outerView->invalidateSizingInfo(View::InvalidateReason::childSizingInfoInvalidated);
                    outerView->needLayout(View::InvalidateReason::childSizingInfoInvalidated);
                }
            }

            void setHorizontalAlignment(const View::HorizontalAlignment &align) override
            {
                // do nothing. The View handles this.
            }

            void setVerticalAlignment(const View::VerticalAlignment &align) override
            {
                // do nothing. The View handles this.
            }

            void setPreferredSizeHint(const Size &hint) override
            {
                // nothing to do by default. Most views do not use this.
            }

            void setPreferredSizeMinimum(const Size &limit) override
            {
                // do nothing. The View handles this.
            }

            void setPreferredSizeMaximum(const Size &limit) override
            {
                // do nothing. The View handles this.
            }

            Rect adjustAndSetBounds(const Rect &requestedBounds) override
            {
                // first adjust the bounds so that they are on pixel boundaries
                Rect adjustedBounds = adjustBounds(requestedBounds, RoundType::nearest, RoundType::nearest);

                _view.frame = rectToIosRect(adjustedBounds);

                return adjustedBounds;
            }

            Rect adjustBounds(const Rect &requestedBounds, RoundType positionRoundType,
                              RoundType sizeRoundType) const override
            {
                // most example code for ios simply aligns on integer values
                // when pixel alignment is discussed. While this DOES actually
                // align on a grid that coincides with some pixel boundaries,
                // the actual screen resolution can be much higher (and indeed
                // it is for all modern iPhones). iOS uses an integral scale
                // factor from "points" (=what we call DIPs) to pixels. Aligning
                // to integers aligns to full points. But there can be 1, 2, 3
                // or more actual pixels per point.

                // Aligning to full points has the disadvantage that we do not
                // take full advantage of the display resolution when it comes
                // to positioning things. This can make animations less smooth
                // than they could be.

                // On macOS there is the function backingAlignedRect which can
                // align to a proper boundary in a high level way. Apparently
                // such a function does not exist on iOS. So we have to manually
                // align.

                UIScreen *screen = [UIScreen mainScreen];

                double scale = screen.scale; // 1 for old displays, 2 for retina
                                             // iphone, 3 for iphone plus, etc.

                return Dip::pixelAlign(requestedBounds, scale, positionRoundType, sizeRoundType);
            }

            double uiLengthToDips(const UiLength &uiLength) const override
            {
                switch (uiLength.unit) {
                case UiLength::Unit::none:
                    return 0;

                case UiLength::Unit::dip:
                    return uiLength.value;

                case UiLength::Unit::em:
                    return uiLength.value * getEmSizeDips();

                case UiLength::Unit::sem:
                    return uiLength.value * getSemSizeDips();

                default:
                    throw InvalidArgumentError("Invalid UiLength unit passed to "
                                               "ViewCore::uiLengthToDips: " +
                                               std::to_string((int)uiLength.unit));
                }
            }

            Margin uiMarginToDipMargin(const UiMargin &margin) const override
            {
                return Margin(uiLengthToDips(margin.top), uiLengthToDips(margin.right), uiLengthToDips(margin.bottom),
                              uiLengthToDips(margin.left));
            }

            Size calcPreferredSize(const Size &availableSpace = Size::none()) const override
            {
                Margin padding = getPaddingDips();

                CGSize constraintSize = UILayoutFittingCompressedSize;

                // systemLayoutSizeFittingSize will clip the return value to the
                // constraint size. So we only pass the available space if the
                // view can actually adjust itself to the available space.
                if (std::isfinite(availableSpace.width) && canAdjustToAvailableWidth()) {
                    constraintSize.width = availableSpace.width - (padding.left + padding.right);
                    if (constraintSize.width < 0)
                        constraintSize.width = 0;
                }
                if (std::isfinite(availableSpace.height) && canAdjustToAvailableHeight()) {
                    constraintSize.height = availableSpace.height - (padding.top + padding.bottom);
                    if (constraintSize.height < 0)
                        constraintSize.height = 0;
                }

                CGSize iosSize = [_view systemLayoutSizeFittingSize:constraintSize];

                Size size = iosSizeToSize(iosSize);

                if (size.width < 0)
                    size.width = 0;
                if (size.height < 0)
                    size.height = 0;

                size += padding;

                if (size.width < 0)
                    size.width = 0;
                if (size.height < 0)
                    size.height = 0;

                P<const View> view = getOuterViewIfStillAttached();
                if (view != nullptr) {
                    size.applyMinimum(view->preferredSizeMinimum());
                    size.applyMaximum(view->preferredSizeMaximum());
                }

                return size;
            }

            void layout() override
            {
                // do nothing by default. Most views do not have subviews.
            }

            bool canMoveToParentView(View &newParentView) const override { return true; }

            void moveToParentView(View &newParentView) override
            {
                P<View> outer = getOuterViewIfStillAttached();
                if (outer != nullptr) {
                    P<View> parent = outer->getParentView();

                    if (&newParentView != parent.getPtr()) {
                        // Parent has changed. Remove the view from its current
                        // super view.
                        dispose();
                        _addToParent(&newParentView);
                    }
                }
            }

            void dispose() override { removeFromUISuperview(); }

            virtual void addChildUIView(UIView *childView) { [_view addSubview:childView]; }

            virtual void removeFromUISuperview() { [_view removeFromSuperview]; }

          protected:
            /** Returns the default padding for the control.
                The default implementation returns zero-padding.*/
            virtual Margin getDefaultPaddingDips() const { return Margin(); }

            Margin getPaddingDips() const
            {
                // add the padding
                Margin padding;

                Nullable<UiMargin> pad;
                P<const View> view = getOuterViewIfStillAttached();
                if (view != nullptr)
                    pad = view->padding();

                if (pad.isNull())
                    padding = getDefaultPaddingDips();
                else
                    padding = uiMarginToDipMargin(pad.get());

                return padding;
            }

            /** Returns true if the view can adjust its size to fit into a given
                width.
                The default return value is false. Derived view classes can
               override this to indicate that they can adapt.*/
            virtual bool canAdjustToAvailableWidth() const { return false; }

            /** Returns true if the view can adjust its size to fit into a given
                height.
                The default return value is false. Derived view classes can
               override this to indicate that they can adapt.*/
            virtual bool canAdjustToAvailableHeight() const { return false; }

          private:
            void _addToParent(View *parentView)
            {
                if (parentView == nullptr) {
                    // top level window. Nothing to do.
                    return;
                }

                P<IViewCore> parentCore = parentView->getViewCore();
                if (parentCore == nullptr) {
                    // this should not happen. The parent MUST have a core -
                    // otherwise we cannot initialize ourselves.
                    throw ProgrammingError("bdn::ios::ViewCore constructed for a view whose "
                                           "parent does not have a core.");
                }

                cast<ViewCore>(parentCore)->addChildUIView(_view);
            }

            virtual double getFontSize() const
            {
                // most views do not have a font size attached to them on ios.
                // UiLabel and UiButton are pretty much the only ones.
                // Those should override this function.
                // In the default implementation we simply return the system
                // font size.
                return getSemSizeDips();
            }

            double getEmSizeDips() const
            {
                if (_emDipsIfInitialized == -1)
                    _emDipsIfInitialized = getFontSize();

                return _emDipsIfInitialized;
            }

            double getSemSizeDips() const
            {
                if (_semDipsIfInitialized == -1)
                    _semDipsIfInitialized = UiProvider::get().getSemSizeDips();

                return _semDipsIfInitialized;
            }

            WeakP<View> _outerViewWeak;

            UIView *_view;

            mutable double _emDipsIfInitialized = -1;
            mutable double _semDipsIfInitialized = -1;
        };
    }
}

#endif
