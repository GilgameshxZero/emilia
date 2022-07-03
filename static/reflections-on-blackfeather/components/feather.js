import { registerComponent } from "../component.js";

registerComponent(
	`feather`,
	class extends HTMLElement {
		constructor() {
			super();
			this.onanimationend = this.remove;
		}

		onComponentDOMContentLoaded() {
			// Prevents flash on upper-left corner during creation.
			this.componentLoad.then(() => {
				this.style.setProperty(
					`--travel-begin-top`,
					`${20 + Math.random() * 30}%`
				);
				this.style.setProperty(
					`--travel-end-left`,
					`${80 + Math.random() * 80}%`
				);
				this.style.setProperty(`--time-travel`, `${20 + Math.random() * 20}s`);
				this.style.setProperty(`--time-spin`, `${8 + Math.random() * 8}s`);
				this.style.setProperty(
					`--spin-direction`,
					`${Math.random() < 0.5 ? -1 : 1}`
				);
				this.setAttribute(`active`, ``);
			});
		}
	}
);
