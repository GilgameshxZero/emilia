import { getCookie } from "./cookie.js";

export function resolveStoryworld() {
	const urlParams = new URLSearchParams(window.location.search);
	const storyworldForced = urlParams.get(`storyworld`);
	if (storyworldForced) {
		urlParams.delete(`storyworld`);
		const queryParamsStr = urlParams.toString();
		const queryParamsDelim = queryParamsStr.length == 0 ? `` : `?`;
		window.history.pushState(
			null,
			``,
			`${window.location.pathname}${queryParamsDelim}${queryParamsStr}`
		);
		return storyworldForced;
	}

	const storyworldSelected = getCookie(`storyworld`);
	if (storyworldSelected) {
		return storyworldSelected;
	}

	return window.matchMedia(`(prefers-color-scheme: dark)`).matches
		? `erlija-past` // `reflections-on-blackfeather`
		: `erlija-past`;
}

export function loadStoryworld(storyworld) {
	import(`../${storyworld}/components/landing.js`).then(() => {
		const landing = document.createElement(`emilia-landing`);
		document.body.appendChild(landing);
		landing.resourceLoad.then(() => {
			landing.subcomponentLoad.then(() => {
				document.body.classList.add(`loaded`);
			});
		});
	});
}
