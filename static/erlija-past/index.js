// When this script is executed, the pre-loader is already displayed.
import { createSceneFromUrl } from "./url.js";
import { maybeLoadDashboard } from "./dashboard.js";
import "./components/dashboard.js";
import "./components/sunset.js";
import "./components/splash.js";

// This event is only fired when all non-deferred scripts and stylesheets on index.html have been loaded and executed, including imported scripts.
window.addEventListener(
	`load`,
	() => {
		if (maybeLoadDashboard()) {
			return;
		}

		// Wait for all immediately accessible components to load.
		const splash = document.querySelector(`emilia-splash`);
		Promise.all([
			splash.componentLoad,
			document.querySelector(`emilia-sunset`).componentLoad
		]).then(() => {
			document.body.removeAttribute(`pre-load`);
			splash.sceneIn();
		});

		// Pressing the back button loads a scene based on the new url.
		window.addEventListener(`popstate`, () => {
			document.body.removeAttribute(`active`);
			const oldScene = document.querySelector(`body>:last-child`);
			const newScene = createSceneFromUrl(window.location.search);
			setTimeout(() => {
				document.body.removeChild(oldScene);
				newScene.sceneIn();
			}, 500);
		});
	},
	{ once: true }
);
