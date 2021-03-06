/*
 * Copyright 2008-09, Oliver Ruiz Dorantes, <oliver.ruiz.dorantes_at_gmail.com>
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef BLUETOOTH_WINDOW_H
#define BLUETOOTH_WINDOW_H

#include "BluetoothSettingsView.h"

#include <Application.h>
#include <Button.h>
#include <Window.h>
#include <Message.h>
#include <TabView.h>

class BluetoothSettingsView;
class RemoteDevicesView;

class BluetoothWindow : public BWindow {
public:
			BluetoothWindow(BRect frame);
	bool	QuitRequested();
	void	MessageReceived(BMessage *message);

private:
			RemoteDevicesView*		fRemoteDevices;
			BButton*				fDefaultsButton;
			BButton*				fRevertButton;
			BMenuBar*				fMenubar;

			BluetoothSettingsView*	fSettingsView;
};

#endif
