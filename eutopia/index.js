// When this script is executed, the pre-loader is already displayed.
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
