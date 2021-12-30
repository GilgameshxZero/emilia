import { registerComponent } from "../components.js";
import "./storyworld-portal.js";
import "./neon.js";

registerComponent(
	`arx`,
	class extends HTMLElement {
		constructor() {
			super();
		}

		onComponentDOMContentLoaded() {
			this.componentLoad.then(() => {
				this.shadowRoot
					.querySelector(`emilia-neon`)
					.addEventListener(`click`, () => {
						document.querySelector(`emilia-underlay`).toCoast();
					});
			});
		}
	}
);
