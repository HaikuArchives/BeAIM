#ifndef _RT_ENGINE_H_
#define _RT_ENGINE_H_

#include <View.h>
#include <Font.h>

class RTEngine : public BView {

	public:
		RTEngine( BRect frame );
		void Draw(BRect updateRect);
		
	private:
		BFont font;
		int leftMargin;
		int rightMargin;
};

#endif
