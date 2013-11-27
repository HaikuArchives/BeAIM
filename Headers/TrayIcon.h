#ifndef _TRAY_ICON_H_
#define _TRAY_ICON_H_

#ifdef _EXPORT
class _EXPORT TrayIcon;
#endif

#include <Bitmap.h>
#include <View.h>

class TrayIcon : public BView
{
	public:
	
		// constructors - second one is a BArchivable constructor
		TrayIcon( BRect frame, BBitmap* active, BBitmap* inactive );
		TrayIcon( BMessage *archive );
		
		// destructor
		virtual ~TrayIcon();
		
		// adds and takes away the icon
		static void AddTrayIcon();
		static void RemoveTrayIcon();
		
		// standard handlers
		virtual void MessageReceived(BMessage *);		
		virtual void AttachedToWindow(void);
		virtual void Draw( BRect updateRect );
		virtual void MouseDown(BPoint);	
		
		// replicant stuff
		virtual status_t Archive( BMessage *archive, bool=true ) const;
		static TrayIcon *Instantiate( BMessage *archive );
		virtual BHandler *RecipientHandler() const { return (BHandler *)this; }
		
	private:
		
		BBitmap* ActiveIcon;
		BBitmap* InactiveIcon;
};

#endif
