import { registerComponent } from "../components.js";

registerComponent(
	`storyworld-portal`,
	class extends HTMLElement {
		constructor() {
			super();

			this.onclick = () => {
				document.body.setAttribute(`pre-load`, ``);
				// Set storyworld cookie for 4 weeks.
				document.cookie = `storyworld-selected=${this.getAttribute(
					`storyworld`
				)}; Max-Age=2419200`;
				setTimeout(() => {
					window.location.pathname = `/`;
				}, 1000);
			};
		}

		onComponentDOMContentLoaded() {
			// Set image source based on storyworld attribute.
			const imgMap = {
				"erlija-past": "sunflower",
				"reflections-on-blackfeather": "blackfeather"
			};
			const storyworld = this.getAttribute(`storyworld`);
			this.shadowRoot
				.querySelector(`img`)
				.setAttribute(`src`, `/${storyworld}/${imgMap[storyworld]}.png`);
		}
	}
);
