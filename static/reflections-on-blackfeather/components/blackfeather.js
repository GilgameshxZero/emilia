import { registerComponent } from "../components.js";
import "./neon.js";
import "./snapshot.js";

registerComponent(
	`blackfeather`,
	class extends HTMLElement {
		constructor() {
			super();
		}

		onComponentDOMContentLoaded() {
			this.shadowRoot
				.querySelector(`emilia-neon`)
				.addEventListener(`click`, () => {
					document.querySelector(`emilia-underlay`).toCoast();
				});
		}

		loadSnapshot(snapshotName) {
			this.shadowRoot
				.querySelector(`emilia-snapshot`)
				.loadSnapshot(snapshotName);
			this.shadowRoot.querySelector(`emilia-snapshot`).scrollTop = 0;
		}
	}
);
