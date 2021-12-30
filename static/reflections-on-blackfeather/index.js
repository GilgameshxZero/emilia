// When this script is executed, the pre-loader is already displayed.
import "./components/underlay.js";

// This event is only fired when all non-deferred scripts and stylesheets on index.html have been loaded and executed, including imported scripts.
window.addEventListener(
	`load`,
	() => {
		// Wait for all immediately accessible components to load.
		const underlay = document.querySelector(`emilia-underlay`);
		Promise.all([underlay.componentLoad]).then(() => {
			document.body.removeAttribute(`pre-load`);
		});
	},
	{ once: true }
);
