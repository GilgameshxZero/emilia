var minSocialPanelWidth = 250;

var html;
var homeMainInner;
var homeSocial;

function onLoad() {
	html = document.getElementsByTagName('html')[0];
	homeMainInner = document.getElementById("home-main-inner");
	homeSocial = document.getElementById("home-social");

	new ResizeObserver(onHomeResize).observe(homeMainInner);
	new ResizeObserver(onHomeSocialResize).observe(homeSocial);
}
onLoad();

function onHomeResize() {
	var scrollWidth = homeMainInner.offsetWidth - homeMainInner.clientWidth;

	console.log(homeMainInner.offsetWidth, homeMainInner.clientWidth);
	homeMainInner.style.width = "calc(100% + " + scrollWidth + "px";
}

function onHomeSocialResize() {
	html.style.setProperty("--social-panel-cols", Math.min(4, Math.ceil(html.clientWidth / minSocialPanelWidth)));
}