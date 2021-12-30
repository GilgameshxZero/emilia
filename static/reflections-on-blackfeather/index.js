// When this script is executed, the pre-loader is already displayed.
import "./components/underlay.js";

// This event is only fired when all non-deferred scripts and stylesheets on index.html have been loaded and executed, including imported scripts.
window.addEventListener(
	`load`,
	() => {
		const underlay = document.querySelector(`emilia-underlay`);

		let locationName;
		if (window.location.pathname.match(/\/storyworlds\/?/g)) {
			locationName = `arx`;
		} else if (window.location.pathname.match(/\/snapshots\/[^\/\?#]+/g)) {
			locationName = `blackfeather`;
		} else {
			locationName = `coast`;
		}
		underlay.setAttribute(`location`, locationName);

		// Wait for all immediately accessible components to load.
		Promise.all([underlay.componentLoad]).then(() => {
			if (locationName === `blackfeather`) {
				underlay.toBlackfeather(
					window.location.pathname.match(/\/snapshots\/([^\/\?#]+)/)[1]
				);
			}
			document.body.removeAttribute(`pre-load`);
		});
	},
	{ once: true }
);
