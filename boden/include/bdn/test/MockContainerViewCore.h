#ifndef BDN_TEST_MockContainerViewCore_H_
#define BDN_TEST_MockContainerViewCore_H_


#include <bdn/test/MockViewCore.h>

#include <bdn/ContainerView.h>

#include <bdn/ProgrammingError.h>

namespace bdn
{
namespace test
{


/** Implementation of a "fake" container view core that does not actually show anything
    visible, but behaves otherwise like a normal container view core.
    
    See MockUiProvider.
    */
class MockContainerViewCore : public MockViewCore
{
public:
	MockContainerViewCore(ContainerView* pView)
		: MockViewCore(pView)
	{
	}
    
	
    Size calcPreferredSize() const
    {
	    // this core function should never have been called.
	    // The outer window is responsible for everything layout-related.
	    programmingError("ContainerView::calcPreferredSize must be overloaded in derived class.");
        return Size();
    }

    int calcPreferredHeightForWidth(int width) const
    {
	    // this core function should never have been called.
	    // The outer window is responsible for everything layout-related.
	    programmingError("ContainerView::calcPreferredHeightForWidth must be overloaded in derived class.");
        return 0;
    }

    int calcPreferredWidthForHeight(int height) const
    {
	    // this core function should never have been called.
	    // The outer window is responsible for everything layout-related.
	    programmingError("ContainerView::calcPreferredWidthForHeight must be overloaded in derived class.");
        return 0;
    }


};

}
}

#endif