// When this script is executed, the pre-loader is already displayed.
import "./component/sunset.js";
import "./component/splash.js";

// This event is only fired when all scripts and stylesheets on index.html have been loaded and executed, including imported scripts.
window.addEventListener(
	`load`,
	() => {
		// Defer loading some stylesheets until here, so that pre-loader can be shown as early as possible.
		const styles = [`common/font.css`];
		const styleElements = styles.map((style) => {
			const element = document.createElement(`link`);
			element.setAttribute(`rel`, `stylesheet`);
			element.setAttribute(`href`, style);
			return element;
		});
		const styleLoads = styleElements.map((element) => {
			return new Promise((resolve, reject) => {
				element.addEventListener(`load`, () => {
					resolve();
				});
			});
		});

		// Begin loading deferred stylesheets.
		styleElements.forEach((element) => {
			document.head.appendChild(element);
		});

		// Wait for all immediately accessible components to load.
		const splash = document.querySelector(`eutopia-splash`);
		Promise.all([
			document.querySelector(`eutopia-sunset`).componentLoad,
			splash.componentLoad,
			...styleLoads
		]).then(() => {
			document.body.removeAttribute(`pre-load`);
			splash.sceneIn();
		});
	},
	{ once: true }
);
