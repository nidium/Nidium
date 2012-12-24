/* -------------------------- */
/* Native (@) 2013 Stight.com */
/* -------------------------- */

var ಠ_ಠ = '2.0',
	
	__PATH_ROOT__ = 'falcon/',
	__PATH_KERNEL__ = __PATH_ROOT__ + 'kernel/',
	__PATH_LAYOUT__ = __PATH_ROOT__ + 'layout/',
	__PATH_PLUGINS__ = __PATH_ROOT__ + 'plugins/';

/* -------------------------------------------------------------------------- */

/* Todo : add more performance flags */
var	__ENABLE_TEXT_SHADOWS__ = true,
	__ENABLE_BUTTON_SHADOWS__ = true,
	__ENABLE_GRADIENT_LAYERS__ = true,
	__DEBUG_SHOW_LAYERS__ = false;

/* -------------------------------------------------------------------------- */

load(__PATH_KERNEL__ + 'helper.inc.js');
load(__PATH_KERNEL__ + 'engine.inc.js');
load(__PATH_KERNEL__ + 'layout.inc.js');
load(__PATH_KERNEL__ + 'events.inc.js');
load(__PATH_KERNEL__ + 'motion.inc.js');

/* -- UI Elements -- */
load(__PATH_LAYOUT__ + 'UIView.js');
load(__PATH_LAYOUT__ + 'UIButton.js');
load(__PATH_LAYOUT__ + 'UIButtonClose.js');
load(__PATH_LAYOUT__ + 'UITab.js');
load(__PATH_LAYOUT__ + 'UITabController.js');

/* -- Canvas Plugins -- */
load(__PATH_PLUGINS__ + 'misc.inc.js');
load(__PATH_PLUGINS__ + 'roundbox.inc.js');
load(__PATH_PLUGINS__ + 'tabbox.inc.js');

/* -------------------------------------------------------------------------- */

