import { registerComponent } from "../components.js";
import "./neon.js";

registerComponent(
	`lighthouse`,
	class extends HTMLElement {
		constructor() {
			super();
		}

		onComponentDOMContentLoaded() {
			this.componentLoad.then(() => {
				this.addEventListener(`click`, () => {
					document
						.querySelector(`emilia-underlay`)
						.toBlackfeather(this.getAttribute(`name`));
				});
				this.addEventListener(`mouseover`, () => {
					this.shadowRoot
						.querySelector(`emilia-neon`)
						.setAttribute(`active`, ``);
				});
				this.addEventListener(`mouseout`, () => {
					this.shadowRoot
						.querySelector(`emilia-neon`)
						.removeAttribute(`active`);
				});
			});
		}
	}
);
