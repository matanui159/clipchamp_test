#include <emscripten.h>
#include "encoder.h"
#include "display.h"

static int g_offset = 0;

static void loop() {
	int data_size = EM_ASM_INT(return data_size);
	byte_t* data = (byte_t*)EM_ASM_INT(return data);
	int width = EM_ASM_INT(return id("width").value);
	int height = EM_ASM_INT(return id("height").value);
	int fps = EM_ASM_INT(return id("fps").value);

	int frame_size = width * height;
	frame_size += frame_size / 2;
	int offset = g_offset++ * frame_size;
	if (offset + frame_size > data_size) {
		offset = g_offset = 0;
	}

	if (width > 0 && height > 0 && fps > 0 && data != NULL) {
		EM_ASM(id("button").disabled = false);
		emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 1000 / fps);
		display_frame(width, height, data + offset);
	} else {
		EM_ASM(id("button").disabled = true);
		emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
	}
}

static void download(int size, const byte_t* data) {
	EM_ASM({
		var size = $0;
		var data = HEAPU8.slice($1, $1 + size);
		var blob = new Blob([data], {type: "octet/stream"});
		var url = window.URL.createObjectURL(blob);
		id("download").href = url;
		id("download").click();
		window.URL.revokeObjectURL(url);
	}, size, data);
}

void convert(int width, int height, int fps, int size, byte_t* data) {
	int frame_size = width * height;
	frame_size += frame_size / 2;
	encoder_t* enc = encoder_create(width, height, fps);

	while (size >= frame_size) {
		encoder_frame(enc, data);
		size -= frame_size;
		data += frame_size;
	}

	int out_size;
	const byte_t* out = encoder_data(enc, &out_size);
	download(out_size, out);
	encoder_destroy(enc);
}

int main() {
	EM_ASM(
		id = function(i) {
			return document.getElementById(i);
		};

		data_size = 0;
		data = 0;
		var reader = new FileReader();
		reader.onload = function() {
			var array = new Uint8Array(reader.result);
			data_size = array.byteLength;
			data = _malloc(data_size);
			HEAPU8.set(array, data);
		};

		id("file").onchange = function() {
			_free(data);
			data = 0;
			var file = id("file").files[0];
			if (file) {
				id("download").download = file.name + ".264";
				reader.readAsArrayBuffer(file);
			}
		};

		id("button").onclick = function() {
			_convert(
				id("width").value,
				id("height").value,
				id("fps").value,
				data_size,
				data
			);
		};

		Module.canvas = id("canvas");
		Module.doNotCaptureKeyboard = true;
	);

	display_init();
	emscripten_set_main_loop(loop, 0, 0);
	return 0;
}