#include "PlotWidget.hpp"
#include "Analyzer.hpp"
#include <cairo.h>
#include <algorithm>
#include <vector>
#include <random>
#include <glib.h>

G_DEFINE_TYPE(PlotWidget, plot_widget, GTK_TYPE_WIDGET)

static void plot_widget_snapshot(GtkWidget *widget, GtkSnapshot *snapshot) {
    PlotWidget *self = PLOT_WIDGET(widget);
    if (self->original_signal.empty() || self->noisy_signal.empty()) {
        return;
    }

    double width = gtk_widget_get_width(widget);
    double height = gtk_widget_get_height(widget);
    graphene_rect_t rect = GRAPHENE_RECT_INIT(0, 0, (float)width, (float)height);
    cairo_t *cr = gtk_snapshot_append_cairo(snapshot, &rect);

    // Background
    cairo_set_source_rgb(cr, 0.878, 0.878, 0.878); // #E0E0E0
    cairo_paint(cr);

    if (self->plot_type == PLOT_TYPE_SIGNAL) {
        // Signal plot
        double max_val = *std::max_element(self->original_signal.begin(), self->original_signal.end());
        double min_val = *std::min_element(self->original_signal.begin(), self->original_signal.end());
        double noisy_max = *std::max_element(self->noisy_signal.begin(), self->noisy_signal.end());
        double noisy_min = *std::min_element(self->noisy_signal.begin(), self->noisy_signal.end());
        max_val = std::max(max_val, noisy_max);
        min_val = std::min(min_val, noisy_min);
        double range = max_val - min_val;
        if (range == 0) range = 1.0;

        // Original signal
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 2.0);
        for (size_t i = 0; i < self->original_signal.size(); ++i) {
            double x = (i * width) / self->original_signal.size();
            double y = height - ((self->original_signal[i] - min_val) / range) * height * 0.8 - height * 0.1;
            if (i == 0) {
                cairo_move_to(cr, x, y);
            } else {
                cairo_line_to(cr, x, y);
            }
        }
        cairo_stroke(cr);

        // Noisy signal
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        for (size_t i = 0; i < self->noisy_signal.size(); ++i) {
            double x = (i * width) / self->noisy_signal.size();
            double y = height - ((self->noisy_signal[i] - min_val) / range) * height * 0.8 - height * 0.1;
            if (i == 0) {
                cairo_move_to(cr, x, y);
            } else {
                cairo_line_to(cr, x, y);
            }
        }
        cairo_stroke(cr);

        // Axes
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 2.0);
        cairo_move_to(cr, 0, height / 2);
        cairo_line_to(cr, width, height / 2);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0, height);
        cairo_stroke(cr);

        // Legend
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        cairo_rectangle(cr, 10, 10, 20, 10);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 40, 20);
        cairo_show_text(cr, "Original Signal");
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_rectangle(cr, 10, 30, 20, 10);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 40, 40);
        cairo_show_text(cr, "Noisy Signal");
    } else if (self->plot_type == PLOT_TYPE_TIME) {
        // Time domain plot
        double max_val = *std::max_element(self->noisy_signal.begin(), self->noisy_signal.end());
        double min_val = *std::min_element(self->noisy_signal.begin(), self->noisy_signal.end());
        double range = max_val - min_val;
        if (range == 0) range = 1.0;

        // Noisy signal
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_set_line_width(cr, 2.0);
        for (size_t i = 0; i < self->noisy_signal.size(); ++i) {
            double x = (i * width) / self->noisy_signal.size();
            double y = height - ((self->noisy_signal[i] - min_val) / range) * height * 0.8 - height * 0.1;
            if (i == 0) {
                cairo_move_to(cr, x, y);
            } else {
                cairo_line_to(cr, x, y);
            }
        }
        cairo_stroke(cr);

        // Zero crossings
        Analyzer analyzer;
        auto crossing_points = analyzer.computeZeroCrossingPoints(self->noisy_signal);
        for (const auto& idx : crossing_points) {
            double x = (idx * width) / self->noisy_signal.size();
            double y = height / 2; // Zero line
            cairo_set_source_rgb(cr, 0.0, 1.0, 0.0); // Green for crossings
            cairo_arc(cr, x, y, 3.0, 0, 2 * G_PI);
            cairo_fill(cr);
        }

        // Axes
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 2.0);
        cairo_move_to(cr, 0, height / 2);
        cairo_line_to(cr, width, height / 2);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 0, height);
        cairo_stroke(cr);

        // Legend
        cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_rectangle(cr, 10, 10, 20, 10);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 40, 20);
        cairo_show_text(cr, "Noisy Signal");
        cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        cairo_rectangle(cr, 10, 30, 10, 10);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 40, 40);
        cairo_show_text(cr, "Zero Crossings");
    } else if (self->plot_type == PLOT_TYPE_PHASOR) {
        // Phasor plot
        std::vector<double> noise(self->noisy_signal.size());
        for (size_t i = 0; i < self->noisy_signal.size(); ++i) {
            noise[i] = self->noisy_signal[i] - self->original_signal[i];
        }
        double noisePower = std::accumulate(noise.begin(), noise.end(), 0.0,
            [](double sum, double x) { return sum + x * x; }) / noise.size();
        double sigma = std::sqrt(noisePower / 2);

        std::mt19937 rng(self->seed);
        std::normal_distribution<> dist(0.0, sigma);
        std::vector<double> real_parts(noise.size()), imag_parts(noise.size());
        for (size_t i = 0; i < noise.size(); ++i) {
            real_parts[i] = dist(rng);
            imag_parts[i] = dist(rng);
        }

        // Plot distribution
        double max_val = 3 * sigma;
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        for (size_t i = 0; i < real_parts.size(); ++i) {
            double x = (width / 2) + (real_parts[i] / max_val) * (width / 3);
            double y = (height / 2) - (imag_parts[i] / max_val) * (height / 2);
            cairo_arc(cr, x, y, 2, 0, 2 * G_PI);
            cairo_fill(cr);
        }

        // Draw circles
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_set_line_width(cr, 2.0);
        for (int i = 1; i <= 3; ++i) {
            double radius = (i * sigma / max_val) * (width / 3);
            cairo_arc(cr, width / 2, height / 2, radius, 0, 2 * G_PI);
            cairo_stroke(cr);
        }

        // Draw axes
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, width / 2, 0);
        cairo_line_to(cr, width / 2, height);
        cairo_move_to(cr, 0, height / 2);
        cairo_line_to(cr, width, height / 2);
        cairo_stroke(cr);

        // Legend
        cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        cairo_rectangle(cr, 10, 40, 20, 10);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, 40, 20);
        cairo_show_text(cr, "Noise Phasor");
        cairo_move_to(cr, 40, 40);
        cairo_show_text(cr, "1σ, 2σ, 3σ Circles");
    }

    cairo_destroy(cr);
}

static void plot_widget_class_init(PlotWidgetClass *klass) {
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->snapshot = plot_widget_snapshot;
}

static void plot_widget_init(PlotWidget *self) {
    gtk_widget_set_size_request(GTK_WIDGET(self), 600, 400);
    gtk_widget_set_vexpand(GTK_WIDGET(self), TRUE);
    self->plot_type = PLOT_TYPE_SIGNAL;
    self->seed = 0;
}

PlotWidget* plot_widget_new() {
    return PLOT_WIDGET(g_object_new(PLOT_WIDGET_TYPE, NULL));
}

void plot_widget_set_data(PlotWidget *self, const std::vector<double>& original, const std::vector<double>& noisy, PlotType plot_type, unsigned int seed) {
    self->original_signal = original;
    self->noisy_signal = noisy;
    self->plot_type = plot_type;
    self->seed = seed;
}