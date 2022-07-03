import { registerComponent } from "../component.js";
import "./neon.js";
import "./snapshot.js";

registerComponent(
	`blackfeather`,
	class extends HTMLElement {
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
