use super::{OutputMode, render_output};
use infekt_core as core;
use std::fmt::Write as _;
use std::fs;
use std::path::{Path, PathBuf};

const MAX_REPORTED_FAILURES: usize = 20;

#[test]
fn regression_samples_match_expected_utf8_output() {
    let samples_dir = Path::new(env!("CARGO_MANIFEST_DIR")).join("../../regression_samples");
    let inputs = discover_inputs(&samples_dir).unwrap_or_else(|err| {
        panic!(
            "failed to discover regression samples in `{}`: {err}",
            samples_dir.display()
        )
    });

    assert!(
        !inputs.is_empty(),
        "no regression sample inputs found in `{}`",
        samples_dir.display()
    );

    let mut failures = Vec::new();

    for input_path in &inputs {
        if let Some(failure) = check_sample(input_path) {
            failures.push(failure);
        }
    }

    assert!(
        failures.is_empty(),
        "{}",
        format_failure_report(inputs.len(), &failures)
    );
}

fn discover_inputs(samples_dir: &Path) -> Result<Vec<PathBuf>, String> {
    let mut inputs = Vec::new();
    collect_inputs(samples_dir, &mut inputs)?;
    inputs.sort();
    Ok(inputs)
}

fn collect_inputs(dir: &Path, inputs: &mut Vec<PathBuf>) -> Result<(), String> {
    for entry in fs::read_dir(dir).map_err(|err| format!("{}: {err}", dir.display()))? {
        let entry = entry.map_err(|err| format!("{}: {err}", dir.display()))?;
        let path = entry.path();
        let file_type = entry
            .file_type()
            .map_err(|err| format!("{}: {err}", path.display()))?;

        if file_type.is_dir() {
            collect_inputs(&path, inputs)?;
        } else if file_type.is_file() && path.extension().is_some_and(|ext| ext == "in") {
            inputs.push(path);
        }
    }

    Ok(())
}

fn check_sample(input_path: &Path) -> Option<SampleFailure> {
    let expected_path = input_path.with_extension("expected-utf8.out");

    if !expected_path.is_file() {
        return Some(SampleFailure::MissingExpected {
            input_path: input_path.to_path_buf(),
            expected_path,
        });
    }

    let expected_bytes = match fs::read(&expected_path) {
        Ok(bytes) => bytes,
        Err(err) => {
            return Some(SampleFailure::ReadExpected {
                input_path: input_path.to_path_buf(),
                expected_path,
                message: err.to_string(),
            });
        }
    };

    let mut nfo_data = core::nfo_data::NfoData::new();
    if let Err(err) = nfo_data.load_from_file(input_path) {
        return Some(SampleFailure::LoadInput {
            input_path: input_path.to_path_buf(),
            message: err,
        });
    }

    let rendered = match render_output(OutputMode::Utf8ClassicText, &nfo_data, None) {
        Ok(output) => output.bytes,
        Err(err) => {
            return Some(SampleFailure::Render {
                input_path: input_path.to_path_buf(),
                message: err,
            });
        }
    };

    if rendered == expected_bytes {
        None
    } else {
        Some(SampleFailure::Mismatch {
            input_path: input_path.to_path_buf(),
            expected_path,
            actual_len: rendered.len(),
            expected_len: expected_bytes.len(),
            first_diff: first_diff(&rendered, &expected_bytes),
        })
    }
}

fn first_diff(actual: &[u8], expected: &[u8]) -> FirstDiff {
    for (index, (actual_byte, expected_byte)) in actual.iter().zip(expected).enumerate() {
        if actual_byte != expected_byte {
            return FirstDiff {
                index,
                actual: Some(*actual_byte),
                expected: Some(*expected_byte),
            };
        }
    }

    let index = actual.len().min(expected.len());
    FirstDiff {
        index,
        actual: actual.get(index).copied(),
        expected: expected.get(index).copied(),
    }
}

fn format_failure_report(sample_count: usize, failures: &[SampleFailure]) -> String {
    let mut report = format!(
        "UTF-8 regression sample check failed for {} of {} samples",
        failures.len(),
        sample_count
    );

    for failure in failures.iter().take(MAX_REPORTED_FAILURES) {
        let _ = write!(report, "\n\n{}", failure.format());
    }

    if failures.len() > MAX_REPORTED_FAILURES {
        let _ = write!(
            report,
            "\n\n... and {} more failure(s)",
            failures.len() - MAX_REPORTED_FAILURES
        );
    }

    report
}

enum SampleFailure {
    MissingExpected {
        input_path: PathBuf,
        expected_path: PathBuf,
    },
    ReadExpected {
        input_path: PathBuf,
        expected_path: PathBuf,
        message: String,
    },
    LoadInput {
        input_path: PathBuf,
        message: String,
    },
    Render {
        input_path: PathBuf,
        message: String,
    },
    Mismatch {
        input_path: PathBuf,
        expected_path: PathBuf,
        actual_len: usize,
        expected_len: usize,
        first_diff: FirstDiff,
    },
}

impl SampleFailure {
    fn format(&self) -> String {
        match self {
            Self::MissingExpected {
                input_path,
                expected_path,
            } => format!(
                "{}\n  missing expected UTF-8 output `{}`",
                input_path.display(),
                expected_path.display()
            ),
            Self::ReadExpected {
                input_path,
                expected_path,
                message,
            } => format!(
                "{}\n  failed to read `{}`: {}",
                input_path.display(),
                expected_path.display(),
                message
            ),
            Self::LoadInput {
                input_path,
                message,
            } => format!(
                "{}\n  failed to load input: {}",
                input_path.display(),
                message
            ),
            Self::Render {
                input_path,
                message,
            } => format!(
                "{}\n  failed to render UTF-8 output: {}",
                input_path.display(),
                message
            ),
            Self::Mismatch {
                input_path,
                expected_path,
                actual_len,
                expected_len,
                first_diff,
            } => format!(
                "{}\n  expected `{}`\n  actual len: {}, expected len: {}\n  first diff: {}",
                input_path.display(),
                expected_path.display(),
                actual_len,
                expected_len,
                first_diff.format()
            ),
        }
    }
}

struct FirstDiff {
    index: usize,
    actual: Option<u8>,
    expected: Option<u8>,
}

impl FirstDiff {
    fn format(&self) -> String {
        format!(
            "byte {} actual {} expected {}",
            self.index,
            format_byte(self.actual),
            format_byte(self.expected)
        )
    }
}

fn format_byte(byte: Option<u8>) -> String {
    byte.map_or_else(|| "<eof>".to_string(), |byte| format!("0x{byte:02X}"))
}
