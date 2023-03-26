from django.db import models

class Currency(models.Model):
    Date = models.DateField("Date", primary_key=True)
    EUR = models.FloatField(null = False, default = 1.0)
    USD = models.FloatField(null = False, default = 1.0)
    CNY = models.FloatField(null = False, default = 1.0)
    GBP = models.FloatField(null = False, default = 1.0)

    class Meta:
        ordering = ['-Date']
    

class CurrencyType(models.Model):
    pass
