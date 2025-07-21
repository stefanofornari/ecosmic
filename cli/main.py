import click

@click.group()
def cli():
    pass

@cli.command()
def run_algorithm():
    click.echo("Running the algorithm...")

if __name__ == '__main__':
    cli()
