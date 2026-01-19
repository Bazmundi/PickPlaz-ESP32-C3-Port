from manim import (
    Axes,
    BLUE,
    Circle,
    Create,
    DOWN,
    GREEN,
    ORANGE,
    RED,
    RIGHT,
    Scene,
    Text,
    UP,
    VGroup,
    config,
)

from sine_wave_scene import SINTAB

APP_SINE_LEN = len(SINTAB)
SINTAB_MAX = max(SINTAB) if SINTAB else 1
SINE_SPEED = 55  # src/pickplaz_app.c: sine_speed
LED_SAMPLE_INDEX = APP_SINE_LEN // 4  # 90-degree sample in a 256-entry table
LED_OFFSETS = (
    ("LED0", 0, BLUE),
    ("LED1", SINE_SPEED, GREEN),
    ("LED2", SINE_SPEED * 2, ORANGE),
    ("LED3", SINE_SPEED * 3, RED),
)


def shifted_values(offset: int) -> list[int]:
    return [SINTAB[(i + offset) % APP_SINE_LEN] for i in range(APP_SINE_LEN)]

def led_intensity(offset: int) -> float:
    value = SINTAB[(LED_SAMPLE_INDEX + offset) % APP_SINE_LEN]
    return value / SINTAB_MAX


class LedPhaseScene(Scene):
    def construct(self) -> None:
        title = Text("LED Phase Offsets (sine_speed = 55)", font_size=28)
        title.to_edge(UP, buff=0.2)

        led_row = VGroup()
        for name, offset, color in LED_OFFSETS:
            intensity = led_intensity(offset)
            led = Circle(radius=0.22, color=color, stroke_width=2)
            led.set_fill(color, opacity=0.05 + 0.95 * intensity)
            label = Text(name, font_size=18)
            label.next_to(led, DOWN, buff=0.08)
            led_row.add(VGroup(led, label))
        led_row.arrange(RIGHT, buff=0.7)

        rows = VGroup()
        for name, offset, color in LED_OFFSETS:
            axes = Axes(
                x_range=[0, APP_SINE_LEN - 1, 64],
                y_range=[0, 260, 64],
                x_length=8.6,
                y_length=1.0,
                tips=False,
            )
            graph = axes.plot_line_graph(
                list(range(APP_SINE_LEN)),
                shifted_values(offset),
                line_color=color,
                add_vertex_dots=False,
            )
            label = Text(f"{name} (t + {offset})", font_size=20)
            label.next_to(axes, UP, buff=0.1)
            rows.add(VGroup(axes, graph, label))

        rows.arrange(DOWN, buff=0.25)

        content = VGroup(led_row, rows)
        content.arrange(DOWN, buff=0.35)
        max_content_height = config.frame_height - title.height - 0.7
        content.scale_to_fit_height(max_content_height)
        content.next_to(title, DOWN, buff=0.25)

        self.add(title)
        self.add(led_row)
        for axes, graph, label in rows:
            self.add(axes, label)
            self.play(Create(graph), run_time=0.4)
        self.wait(2)
