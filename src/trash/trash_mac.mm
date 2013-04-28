/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#import "trash.h"
#import "Cocoa/Cocoa.h"
#import "Foundation/Foundation.h"

static inline NSString* fromQString(const QString &string)
{
    char* cString = string.toUtf8().data();
    return [[NSString alloc] initWithUTF8String:cString];
}

int NTrash(const QString &path, QString *error)
{
	/*NSMutableArray *urls = [[NSMutableArray alloc] init];
	NSString *string = fromQString(path);
	NSLog(@"%@", string);
	[urls addObject:[NSURL fileURLWithPath:string]];
	[[NSWorkspace sharedWorkspace] recycleURLs:urls comletionHandlerL:nil];*/

	/*[[NSWorkspace sharedWorkspace]
		performFileOperation:NSWorkspaceRecycleOperation
		source:@"/Users/admin/Desktop/"
		destination:@""
		files:[NSArray arrayWithObject:@"replacepath2.py"]
		tag:nil];*/

	FSRef fsRef;
	NSString *string = fromQString(path);
	FSPathMakeRefWithOptions((const UInt8 *)[string fileSystemRepresentation],
							kFSPathMakeRefDoNotFollowLeafSymlink, &fsRef, NULL);
	return FSMoveObjectToTrashSync(&fsRef, NULL, kFSFileOperationDefaultOptions);;
}

/* vim: set ts=4 sw=4: */

