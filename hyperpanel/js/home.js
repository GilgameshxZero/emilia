var minSocialPanelWidth = 100;

var html;
var homeMainInner;
var homeSocial;

function onLoad() {
	html = document.getElementsByTagName('html')[0];
	homeMainInner = document.getElementById("home-main-inner");
	homeSocial = document.getElementById("home-social");

	onHomeMainResize();
	setTimeout(function() {
		onHomeMainResize();
	}, 100);
	
	onSocialResize();
	window.addEventListener("resize", function() {
		onSocialResize();
	});
}
onLoad();

function onHomeMainResize() {
	var scrollWidth = homeMainInner.offsetWidth - homeMainInner.clientWidth;
	homeMainInner.style.width = "calc(100% + " + scrollWidth + "px";
}

function onSocialResize() {
	html.style.setProperty("--social-panel-cols", Math.min(4, Math.ceil(homeSocial.clientWidth / minSocialPanelWidth)));
}