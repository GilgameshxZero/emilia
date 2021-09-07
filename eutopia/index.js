// When this script is executed, the pre-loader is already displayed.
import sceneFromUri from "./common/uri.js";
import "./component/sunset.js";
import "./component/splash.js";

// This event is only fired when all non-deferred scripts and stylesheets on index.html have been loaded and executed, including imported scripts.
window.addEventListener(
	`load`,
	() => {
		// Wait for all immediately accessible components to load.
		const splash = document.querySelector(`eutopia-splash`);
		Promise.all([
			document.querySelector(`eutopia-sunset`).componentLoad,
			splash.componentLoad
		]).then(() => {
			document.body.removeAttribute(`pre-load`);
			splash.sceneIn();
		});
	},
	{ once: true }
);

// TODO: Is this the best place to process this logic?
window.addEventListener(`popstate`, () => {
	// Pressing the back button loads a scene based on the new url.
	// Last selector to avoid removing live-server element.
	document.body.removeAttribute(`active`);
	const oldScene = document.querySelector(`body>:last-child`);
	const newScene = sceneFromUri(window.location.search);
	setTimeout(() => {
		document.body.removeChild(oldScene);
		newScene.sceneIn();
	}, 1000);
});
