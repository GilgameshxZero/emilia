import { registerComponent } from "../component.js";

registerComponent(
	`ping`,
	class extends HTMLElement {
		constructor() {
			super();
			this.onPing = this.onPing.bind(this);
		}

		onComponentDOMContentLoaded() {
			this.componentLoad.then(() => {
				this.span = this.shadowRoot.querySelector(`span`);
				this.onPing();
			});
		}

		onPing() {
			const timeSent = Date.now();
			fetch(`/api/ping`)
				.then(() => {
					this.span.innerText = `${Date.now() - timeSent}ms`;
				})
				.catch(() => {
					this.span.innerText = `error`;
				});
			setTimeout(this.onPing, 1000);
		}
	}
);
