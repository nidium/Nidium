# Copyright 2016 Nidium Inc. All rights reserved.
# Use of this source code is governed by a MIT license
# that can be found in the LICENSE file.

from dokumentor import *

NamespaceDoc( "Locale", "Locale class.",
	NO_Sees,
	NO_Examples
)

FunctionDoc( "Locale.language", "Get the system language.",
	SeesDocs( "Locale.language" ),
	[ExampleDoc("console.log(Locale.language)")],
	IS_Static, IS_Public, IS_Fast,
	NO_Params,
	NO_Returns
)

