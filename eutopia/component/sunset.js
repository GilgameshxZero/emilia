import component from "../common/component.js";

component(
	`sunset`,
	class extends HTMLElement {
		constructor() {
			super();
			// onscroll handlers can be set to onProgress since it is bound correctly.
			this.onProgress = this.onProgress.bind(this);
		}

		// Update progress from a scroll event.
		onProgress(event) {
			// Update sunset based on scroll.
			const progress =
				event.target.scrollTop /
				(event.target.scrollHeight - event.target.clientHeight);
			this.style.setProperty(`--progress`, progress);
		}
	}
);
