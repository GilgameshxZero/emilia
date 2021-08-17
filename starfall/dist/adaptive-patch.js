(function() {
	/* Returns a dictionary, mapping `isOpera`, `isFirefox`, `isSafari`, `isIE`, `isEdge`, `isChrome`, `isEdgeChromium`, `isBlink` to booleans specifying whether we are operating within that browser. */
	function detectBrowser() {
		const isOpera = (!!window.opr && !!opr.addons) || !!window.opera || navigator.userAgent.indexOf(' OPR/') >= 0;
		const isFirefox = typeof InstallTrigger !== 'undefined';
		const isSafari = /constructor/i.test(window.HTMLElement) || (function(p) { return p.toString() === "[object SafariRemoteNotification]"; })(!window['safari'] || (typeof safari !== 'undefined' && safari.pushNotification));
		const isIE = /*@cc_on!@*/ false || !!document.documentMode;
		const isEdge = !isIE && !!window.StyleMedia;
		const isChrome = !!window.chrome;
		const isEdgeChromium = isChrome && (navigator.userAgent.indexOf("Edg") != -1);
		const isBlink = (isChrome || isOpera) && !!window.CSS;
		return {
			"isOpera": isOpera,
			"isFirefox": isFirefox,
			"isSafari": isSafari,
			"isIE": isIE,
			"isEdge": isEdge,
			"isChrome": isChrome,
			"isEdgeChromium": isEdgeChromium,
			"isBlink": isBlink,
		};
	}

	// Remove .vscode-light and .vscode-dark on body if in-browser.
	const browserInfo = detectBrowser();
	if (browserInfo[`isOpera`] || browserInfo[`isFirefox`] || browserInfo[`isSafari`] || browserInfo[`isIE`] || browserInfo[`isEdge`] || browserInfo[`isChrome`] || browserInfo[`isEdgeChromium`] || browserInfo[`isBlink`]) {
		document.body.classList.remove(`vscode-light`, `vscode-dark`);
	}

	// Add .adaptive to the root element.
	document.documentElement.classList.add(`adaptive`);
}())
