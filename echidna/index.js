// When this script is executed, the pre-loader is already displayed.
import { maybeLoadDashboard } from "./scripts/dashboard.js";
import { resolveStoryworld, loadStoryworld } from "./scripts/storyworld.js";

// This event is only fired when all non-deferred scripts and stylesheets on index.html have been loaded and executed, including imported scripts.
window.addEventListener(
	`load`,
	() => {
		if (maybeLoadDashboard()) {
			return;
		}

		loadStoryworld(resolveStoryworld());
	},
	{ once: true }
);
